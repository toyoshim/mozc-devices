// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#ifndef COMMON_USB_HID_DEVICE_H_
#define COMMON_USB_HID_DEVICE_H_

#include "usb_device.h"

#include <cstdint>
#include <string>
#include <vector>

class UsbHidDevice : public UsbDevice {
 public:
  struct HidDescriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bReportDescriptorType;
    uint16_t wDescriptorLength;
  } __attribute__((packed));

  static constexpr uint8_t kHidRequestSetReport = 0x09u;
  static constexpr uint8_t kHidRequestSetIdle = 0x0au;
  static constexpr uint8_t kHidRequestSetProtocol = 0x0bu;

  static constexpr uint8_t kDescriptorTypeHid = 0x21u;
  static constexpr uint8_t kDescriptorTypeReport = 0x22u;

  UsbHidDevice(const DeviceDescriptor& device_descriptor,
               const ConfigurationDescriptor& configuration_descriptor,
               const std::vector<InterfaceDescriptor>& interface_descriptors,
               const std::vector<EndPointDescriptor>& endpoint_descriptors,
               const HidDescriptor& hid_descriptor,
               const std::vector<uint8_t>& report_descriptor,
               const std::vector<std::string>& strings);
  UsbHidDevice(const UsbHidDevice&) = delete;
  UsbHidDevice& operator=(const UsbHidDevice&) = delete;
  ~UsbHidDevice() override = default;

  void Report(uint8_t endpoint, std::span<const uint8_t> report);

 private:
  // Implement UsbDevice.
  void FillConfigurations(std::vector<uint8_t>& buffer) override;
  void GetDescriptor(volatile SetupPacket* setup_packet) override;
  void HandleSetupRequest(volatile SetupPacket* setup_packet) override;

  const ConfigurationDescriptor& configuration_descriptor_;
  const std::vector<InterfaceDescriptor>& interface_descriptors_;
  const std::vector<EndPointDescriptor>& endpoint_descriptors_;
  const HidDescriptor& hid_descriptor_;
  const std::vector<uint8_t>& report_descriptor_;
  std::vector<uint8_t> receive_buffer_;
};

#endif  // COMMON_USB_HID_DEVICE_H_
