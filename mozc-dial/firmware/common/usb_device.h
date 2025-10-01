// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#ifndef COMMON_USB_DEVICE_H_
#define COMMON_USB_DEVICE_H_

#include <cstdint>
#include <span>
#include <string>
#include <vector>

class UsbDevice {
 public:
  struct SetupPacket {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
  } __attribute__((packed));

  struct DeviceDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
  } __attribute__((packed));

  struct ConfigurationDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
  } __attribute__((packed));

  struct InterfaceDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
  } __attribute__((packed));

  struct EndPointDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
  } __attribute__((packed));

  static constexpr uint8_t kDirOut = 0x00u;
  static constexpr uint8_t kDirIn = 0x80u;
  static constexpr uint8_t kTypeClass = 0x20;
  static constexpr uint8_t kRecipientInterface = 0x01;
  static constexpr uint8_t kRecipientEndPoint = 0x02;

  static constexpr uint8_t kRequestClearFeature = 0x01u;
  static constexpr uint8_t kRequestSetAddress = 0x05u;
  static constexpr uint8_t kRequestGetDescriptor = 0x06u;
  static constexpr uint8_t kRequestSetConfiguration = 0x09u;

  static constexpr uint8_t kDescriptorTypeDevice = 0x01u;
  static constexpr uint8_t kDescriptorTypeConfiguration = 0x02u;
  static constexpr uint8_t kDescriptorTypeString = 0x03u;
  static constexpr uint8_t kDescriptorTypeInterface = 0x04u;
  static constexpr uint8_t kDescriptorTypeEndPoint = 0x05u;

  static constexpr uint8_t kEndPointAttributeInterrupt = 0x03u;

  static void HandleInterrupt();

  UsbDevice(const DeviceDescriptor& device_descriptor,
            const ConfigurationDescriptor& configuration_descriptor,
            const std::vector<InterfaceDescriptor>& interface_descriptors,
            const std::vector<EndPointDescriptor>& endpoint_descriptors,
            const std::vector<std::string>& strings);
  UsbDevice(const UsbDevice&) = delete;
  UsbDevice& operator=(const UsbDevice&) = delete;
  virtual ~UsbDevice() = default;

 protected:
  virtual void FillConfigurations(std::vector<uint8_t>& buffer);
  virtual void GetDescriptor(volatile SetupPacket* setup);
  virtual void HandleSetupRequest(volatile SetupPacket* setup);
  virtual void OnCompleteToSend(uint8_t endpoint) {};

  void AcknowledgeOutRequest();
  void Send(uint8_t endpoint, std::span<const uint8_t> data);
  void Receive(uint8_t endpoint, std::span<uint8_t> data);

 private:
  void HandleSetupRequest();
  void HandleBufferStatus();
  void HandleBusReset();

  void SendInternal(uint8_t endpoint);
  void ReceiveInternal(uint8_t endpoint);

  void OnSent(uint8_t endpoint, uint32_t length);
  void OnReceived(uint8_t endpoint, uint32_t length);

  void SetAddress(volatile SetupPacket*);
  void SetConfiguration(volatile SetupPacket*);

  size_t GetMaxPacketSize(uint8_t endpoint);
  uint8_t* GetTransferBuffer(uint8_t endpoint);

  const DeviceDescriptor& device_descriptor_;
  const ConfigurationDescriptor& configuration_descriptor_;
  const std::vector<InterfaceDescriptor>& interface_descriptors_;
  const std::vector<EndPointDescriptor>& endpoint_descriptors_;
  const std::vector<std::string>& strings_;

  uint8_t address_ = 0;
  bool should_set_address_ = false;
  bool ready_ = false;
  std::vector<uint8_t> in_next_pid_ = {0u};
  std::vector<uint8_t> out_next_pid_ = {0u};
  std::vector<uint8_t> setup_buffer_;
  std::vector<std::span<const uint8_t>> sending_data_;
  std::vector<std::span<uint8_t>> receiving_data_;
};

#endif  // COMMON_USB_DEVICE_H_