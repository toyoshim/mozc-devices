// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include "usb_hid_device.h"

UsbHidDevice::UsbHidDevice(
    const DeviceDescriptor& device_descriptor,
    const ConfigurationDescriptor& configuration_descriptor,
    const std::vector<InterfaceDescriptor>& interface_descriptors,
    const std::vector<EndPointDescriptor>& endpoint_descriptors,
    const HidDescriptor& hid_descriptor,
    const std::vector<uint8_t>& report_descriptor,
    const std::vector<std::string>& strings)
    : UsbDevice(device_descriptor,
                configuration_descriptor,
                interface_descriptors,
                endpoint_descriptors,
                strings),
      configuration_descriptor_(configuration_descriptor),
      interface_descriptors_(interface_descriptors),
      endpoint_descriptors_(endpoint_descriptors),
      hid_descriptor_(hid_descriptor),
      report_descriptor_(report_descriptor) {}

void UsbHidDevice::Report(uint8_t endpoint, std::span<const uint8_t> report) {
  Send(endpoint, report);
}

void UsbHidDevice::FillConfigurations(std::vector<uint8_t>& buffer) {
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
  std::span<const uint8_t> hid_span(
      reinterpret_cast<const uint8_t*>(&hid_descriptor_),
      sizeof(hid_descriptor_));
  buffer.insert(buffer.end(), hid_span.begin(), hid_span.end());
  for (const auto& endpoint_descriptor : endpoint_descriptors_) {
    std::span<const uint8_t> endpoint_span(
        reinterpret_cast<const uint8_t*>(&endpoint_descriptor),
        sizeof(endpoint_descriptor));
    buffer.insert(buffer.end(), endpoint_span.begin(), endpoint_span.end());
  }
}

void UsbHidDevice::GetDescriptor(volatile SetupPacket* setup) {
  uint8_t type = setup->wValue >> 8;
  switch (type) {
    case UsbHidDevice::kDescriptorTypeReport:
      Send(0, std::span<const uint8_t>(report_descriptor_));
      break;
    default:
      UsbDevice::GetDescriptor(setup);
      break;
  }
}

void UsbHidDevice::HandleSetupRequest(volatile SetupPacket* setup) {
  uint8_t type =
      setup->bmRequestType & ~(kRecipientInterface | kRecipientEndPoint);
  if (type == (kDirOut | kTypeClass)) {
    switch (setup->bRequest) {
      case kHidRequestSetReport:
        receive_buffer_.resize(setup->wLength);
        Receive(0, receive_buffer_);
        break;
      case kHidRequestSetIdle:
        AcknowledgeOutRequest();
        break;
      case kHidRequestSetProtocol:
        printf("set protocol: %s\n", setup->wValue ? "report" : "boot");
        AcknowledgeOutRequest();
        break;
      default:
        AcknowledgeOutRequest();
        printf("unsupported HID class setup out: $%02x\n", setup->bRequest);
        break;
    }
  } else if (type == (kDirIn | kTypeClass)) {
    AcknowledgeOutRequest();
    printf("unsupported HID class setup in: $%02x\n", setup->bRequest);
  } else {
    UsbDevice::HandleSetupRequest(setup);
  }
}