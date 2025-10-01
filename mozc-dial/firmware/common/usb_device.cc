// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include "usb_device.h"

#include <cstdio>
#include <cstring>
#include <memory>

#include "hardware/irq.h"
#include "hardware/regs/usb.h"
#include "hardware/resets.h"
#include "hardware/structs/usb.h"
#include "pico/types.h"

namespace {

constexpr uint8_t kControlEndpoint = 0u;

UsbDevice* sUsbDevice = nullptr;

}  // namespace

extern "C" void isr_usbctrl(void) {
  UsbDevice::HandleInterrupt();
}

// static
void UsbDevice::HandleInterrupt() {
  uint32_t status = usb_hw->ints;

  if (status & USB_INTS_SETUP_REQ_BITS) {
    static_cast<usb_hw_t*>(hw_clear_alias_untyped(usb_hw))->sie_status =
        USB_SIE_STATUS_SETUP_REC_BITS;
    sUsbDevice->HandleSetupRequest();
  }
  if (status & USB_INTS_BUFF_STATUS_BITS) {
    sUsbDevice->HandleBufferStatus();
  }
  if (status & USB_INTS_BUS_RESET_BITS) {
    static_cast<usb_hw_t*>(hw_clear_alias_untyped(usb_hw))->sie_status =
        USB_SIE_STATUS_BUS_RESET_BITS;
    sUsbDevice->HandleBusReset();
  }
}

UsbDevice::UsbDevice(
    const DeviceDescriptor& device_descriptor,
    const ConfigurationDescriptor& configuration_descriptor,
    const std::vector<InterfaceDescriptor>& interface_descriptors,
    const std::vector<EndPointDescriptor>& endpoint_descriptors,
    const std::vector<std::string>& strings)
    : device_descriptor_(device_descriptor),
      configuration_descriptor_(configuration_descriptor),
      interface_descriptors_(interface_descriptors),
      endpoint_descriptors_(endpoint_descriptors),
      strings_(strings) {
  hard_assert(sUsbDevice == nullptr);
  sUsbDevice = this;

  reset_unreset_block_num_wait_blocking(RESET_USBCTRL);
  memset(usb_dpram, 0, sizeof(*usb_dpram));

  irq_set_enabled(USBCTRL_IRQ, true);

  usb_hw->muxing = USB_USB_MUXING_TO_PHY_BITS | USB_USB_MUXING_SOFTCON_BITS;
  usb_hw->pwr =
      USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;
  usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;
  usb_hw->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF_BITS;
  usb_hw->inte = USB_INTS_BUFF_STATUS_BITS | USB_INTS_BUS_RESET_BITS |
                 USB_INTS_SETUP_REQ_BITS;

  sending_data_.push_back(std::span<const uint8_t>());
  receiving_data_.push_back(std::span<uint8_t>());

  uint32_t dpram_offset = offsetof(usb_device_dpram_t, epx_data);
  for (size_t i = 0; i < endpoint_descriptors_.size(); ++i) {
    uint32_t control =
        EP_CTRL_ENABLE_BITS | EP_CTRL_INTERRUPT_PER_BUFFER |
        (endpoint_descriptors_[i].bmAttributes << EP_CTRL_BUFFER_TYPE_LSB) |
        dpram_offset;
    if (endpoint_descriptors_[i].bEndpointAddress & kDirIn) {
      usb_dpram->ep_ctrl[i].in = control;
    } else {
      usb_dpram->ep_ctrl[i].out = control;
    }
    dpram_offset += endpoint_descriptors_[i].wMaxPacketSize;
    sending_data_.push_back(std::span<const uint8_t>());
    receiving_data_.push_back(std::span<uint8_t>());
  }
  size_t endpoints = sending_data_.size();
  in_next_pid_.resize(endpoints);
  out_next_pid_.resize(endpoints);

  static_cast<usb_hw_t*>(hw_set_alias_untyped(usb_hw))->sie_ctrl =
      USB_SIE_CTRL_PULLUP_EN_BITS;
}

void UsbDevice::FillConfigurations(std::vector<uint8_t>& buffer) {
  buffer.clear();
  buffer.reserve(configuration_descriptor_.wTotalLength);
  std::span<const uint8_t> config_span(
      reinterpret_cast<const uint8_t*>(&configuration_descriptor_),
      sizeof(configuration_descriptor_));
  buffer.insert(buffer.end(), config_span.begin(), config_span.end());
  for (const auto& interface_descriptor : interface_descriptors_) {
    std::span<const uint8_t> interface_span(
        reinterpret_cast<const uint8_t*>(&interface_descriptor),
        sizeof(interface_descriptor));
    buffer.insert(buffer.end(), interface_span.begin(), interface_span.end());
  }
  for (const auto& endpoint_descriptor : endpoint_descriptors_) {
    std::span<const uint8_t> endpoint_span(
        reinterpret_cast<const uint8_t*>(&endpoint_descriptor),
        sizeof(endpoint_descriptor));
    buffer.insert(buffer.end(), endpoint_span.begin(), endpoint_span.end());
  }
}

void UsbDevice::GetDescriptor(volatile SetupPacket* setup) {
  uint8_t type = setup->wValue >> 8;
  switch (type) {
    case kDescriptorTypeDevice:
      Send(kControlEndpoint, std::span<const uint8_t>(
                  reinterpret_cast<const uint8_t*>(&device_descriptor_),
                  std::min(sizeof(device_descriptor_),
                           static_cast<size_t>(setup->wLength))));
      break;
    case kDescriptorTypeConfiguration: {
      FillConfigurations(setup_buffer_);
      Send(kControlEndpoint, std::span<const uint8_t>(
                  setup_buffer_.data(),
                  std::min(setup_buffer_.size(),
                           static_cast<size_t>(setup->wLength))));
      break;
    }
    case kDescriptorTypeString: {
      uint8_t index = setup->wValue & 0xff;
      setup_buffer_.clear();
      if (index == 0) {
        setup_buffer_.reserve(4);
        setup_buffer_.push_back(0x04);
        setup_buffer_.push_back(kDescriptorTypeString);
        setup_buffer_.push_back(0x09);
        setup_buffer_.push_back(0x04);
      } else if (index <= strings_.size()) {
        size_t size = strings_[index - 1].size() * 2 + 2;
        setup_buffer_.reserve(size);
        setup_buffer_.push_back(size);
        setup_buffer_.push_back(kDescriptorTypeString);
        for (size_t i = 0; i < strings_[index - 1].size(); ++i) {
          setup_buffer_.push_back(strings_[index - 1][i]);
          setup_buffer_.push_back(0);
        }
      } else {
        setup_buffer_.reserve(2);
        setup_buffer_.push_back(0x02);
        setup_buffer_.push_back(kDescriptorTypeString);
      }
      Send(kControlEndpoint, std::span<const uint8_t>(
                  setup_buffer_.data(),
                  std::min(setup_buffer_.size(),
                           static_cast<size_t>(setup->wLength))));
      break;
    }
    default:
      printf("unsupported get descriptor: $%02x\n", type);
      break;
  }
}

// Note: better to return STALL on unsupported requests.
void UsbDevice::HandleSetupRequest(volatile SetupPacket* setup) {
  uint8_t type =
      setup->bmRequestType & ~(kRecipientInterface | kRecipientEndPoint);
  if (type == kDirOut) {
    switch (setup->bRequest) {
      case kRequestClearFeature:
        AcknowledgeOutRequest();
        break;
      case kRequestSetAddress:
        SetAddress(setup);
        break;
      case kRequestSetConfiguration:
        SetConfiguration(setup);
        break;
      default:
        printf("unsupported setup out: $%02x\n", setup->bRequest);
        break;
    }
  } else if (type == kDirIn) {
    switch (setup->bRequest) {
      case kRequestGetDescriptor:
        GetDescriptor(setup);
        break;
      default:
        printf("unsupported setup in: $%02x\n", setup->bRequest);
        break;
    }
  }
}

void UsbDevice::AcknowledgeOutRequest() {
  Send(kControlEndpoint, std::span<uint8_t>({}));
}

void UsbDevice::Send(uint8_t endpoint, std::span<const uint8_t> data) {
  if (endpoint == kControlEndpoint) {
    in_next_pid_[endpoint] = 1u;
  }
  sending_data_[endpoint] = data;
  SendInternal(endpoint);
}

void UsbDevice::Receive(uint8_t endpoint, std::span<uint8_t> data) {
  receiving_data_[endpoint] = data;
  out_next_pid_[endpoint] = 1u;
  ReceiveInternal(endpoint);
}

void UsbDevice::SendInternal(uint8_t endpoint) {
  size_t transfer_size = std::min(sending_data_[endpoint].size(), GetMaxPacketSize(endpoint));
  uint8_t* buffer = GetTransferBuffer(endpoint);
  memcpy(buffer, sending_data_[endpoint].data(), transfer_size);
  uint32_t pid =
      in_next_pid_[endpoint] ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
  in_next_pid_[endpoint] ^= 1u;
  usb_dpram->ep_buf_ctrl[endpoint].in =
      transfer_size | USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL | pid;

  size_t remaining_size = sending_data_[endpoint].size() - transfer_size;
  if (remaining_size) {
    sending_data_[endpoint] =
        std::span(&sending_data_[endpoint][transfer_size], remaining_size);
  } else {
    sending_data_[endpoint] = std::span<const uint8_t>();
  }
}

void UsbDevice::ReceiveInternal(uint8_t endpoint) {
  size_t transfer_size = std::min(receiving_data_[endpoint].size(), GetMaxPacketSize(endpoint));
  uint32_t pid =
      out_next_pid_[endpoint] ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
  usb_dpram->ep_buf_ctrl[endpoint].out =
      transfer_size | USB_BUF_CTRL_AVAIL | pid;
}

void UsbDevice::HandleSetupRequest() {
  volatile SetupPacket* setup =
      reinterpret_cast<volatile SetupPacket*>(&usb_dpram->setup_packet);
  HandleSetupRequest(setup);
}

void UsbDevice::HandleBufferStatus() {
  uint32_t status = usb_hw->buf_status;
  uint8_t endpoint_bits = (1 + endpoint_descriptors_.size()) * 2;
  for (uint8_t i = 0; i < endpoint_bits; ++i) {
    uint32_t bit = 1 << i;
    uint8_t endpoint = i >> 1;
    bool out = i & 1;
    if ((status & bit) == 0) {
      continue;
    }
    static_cast<usb_hw_t*>(hw_clear_alias_untyped(usb_hw))->buf_status = bit;
    if (out) {
      OnReceived(endpoint,
                 usb_dpram->ep_buf_ctrl[endpoint].out & USB_BUF_CTRL_LEN_MASK);
    } else {
      OnSent(endpoint,
             usb_dpram->ep_buf_ctrl[endpoint].in & USB_BUF_CTRL_LEN_MASK);
    }
  }
}

void UsbDevice::HandleBusReset() {
  address_ = 0;
  ready_ = false;
  usb_hw->dev_addr_ctrl = 0;
  should_set_address_ = false;
  for (auto& data : sending_data_) {
    data = std::span<const uint8_t>();
  }
  for (auto& data : receiving_data_) {
    data = std::span<uint8_t>();
  }
}

void UsbDevice::OnSent(uint8_t endpoint, uint32_t length) {
  if (should_set_address_) {
    usb_hw->dev_addr_ctrl = address_;
    should_set_address_ = false;
  } else if (length == 0) {
    return;
  } else if (sending_data_[endpoint].empty()) {
    Receive(endpoint, {});
    OnCompleteToSend(endpoint);
  } else {
    SendInternal(endpoint);
  }
}

void UsbDevice::OnReceived(uint8_t endpoint, uint32_t length) {
  if (length == 0) {
    return;
  }
  size_t receive_size =
      std::min(receiving_data_[endpoint].size(), static_cast<size_t>(length));
  uint8_t* buffer = GetTransferBuffer(endpoint);
  memcpy(receiving_data_[endpoint].data(), buffer, receive_size);
  size_t remaining_size = receiving_data_[endpoint].size() - receive_size;
  if (remaining_size) {
    receiving_data_[endpoint] =
        std::span(&receiving_data_[endpoint][receive_size], remaining_size);
    ReceiveInternal(endpoint);
  } else {
    receiving_data_[endpoint] = std::span<uint8_t>();
    Send(endpoint, {});
  }
}

void UsbDevice::SetAddress(volatile SetupPacket* setup) {
  address_ = setup->wValue & 0xff;
  should_set_address_ = true;
  AcknowledgeOutRequest();
}

void UsbDevice::SetConfiguration(volatile SetupPacket* setup) {
  ready_ = true;
  AcknowledgeOutRequest();
}

size_t UsbDevice::GetMaxPacketSize(uint8_t endpoint) {
  if (endpoint == kControlEndpoint) {
    return device_descriptor_.bMaxPacketSize0;
  } else if (endpoint <= endpoint_descriptors_.size()) {
    return endpoint_descriptors_[endpoint - 1].wMaxPacketSize;
  }
  hard_assert(false);
  return 0;
}

uint8_t* UsbDevice::GetTransferBuffer(uint8_t endpoint) {
  if (endpoint == kControlEndpoint) {
    return usb_dpram->ep0_buf_a;
  }
  uint8_t* buffer = usb_dpram->epx_data;
  for (size_t i = 1; i < endpoint; ++i) {
    buffer += endpoint_descriptors_[i].wMaxPacketSize;
  }
  return buffer;
}