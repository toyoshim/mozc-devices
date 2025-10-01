// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include "usb_hid_keyboard.h"

namespace {

constexpr uint8_t kManufacturerStringIndex = 1;
constexpr uint8_t kProductStringIndex = 2;
constexpr uint8_t kSerialNumberStringIndex = 3;

constexpr uint8_t kModifierReportIndex = 0;
constexpr uint8_t kKeycodeReportIndex = 2;

UsbDevice::DeviceDescriptor device_descriptor = {
    .bLength = sizeof(UsbDevice::DeviceDescriptor),
    .bDescriptorType = UsbDevice::kDescriptorTypeDevice,
    .bcdUSB = 0x0110,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0,
    .idProduct = 0,
    .bcdDevice = 0,
    .iManufacturer = kManufacturerStringIndex,
    .iProduct = kProductStringIndex,
    .iSerialNumber = kSerialNumberStringIndex,
    .bNumConfigurations = 1};

std::vector<UsbDevice::EndPointDescriptor> endpoint_descriptors = {
    {.bLength = sizeof(UsbDevice::EndPointDescriptor),
     .bDescriptorType = UsbDevice::kDescriptorTypeEndPoint,
     .bEndpointAddress = UsbDevice::kDirIn | 1,
     .bmAttributes = UsbDevice::kEndPointAttributeInterrupt,
     .wMaxPacketSize = 64,
     .bInterval = 10},
    {.bLength = sizeof(UsbDevice::EndPointDescriptor),
     .bDescriptorType = UsbDevice::kDescriptorTypeEndPoint,
     .bEndpointAddress = UsbDevice::kDirOut | 1,
     .bmAttributes = UsbDevice::kEndPointAttributeInterrupt,
     .wMaxPacketSize = 64,
     .bInterval = 10}};

std::vector<UsbDevice::InterfaceDescriptor> interface_descriptors = {
    {.bLength = sizeof(UsbDevice::InterfaceDescriptor),
     .bDescriptorType = UsbDevice::kDescriptorTypeInterface,
     .bInterfaceNumber = 0,
     .bAlternateSetting = 0,
     .bNumEndpoints = static_cast<uint8_t>(endpoint_descriptors.size()),
     .bInterfaceClass = 3,     // HID
     .bInterfaceSubClass = 1,  // Boot
     .bInterfaceProtocol = 1,  // Keyboard
     .iInterface = 0}};

std::vector<uint8_t> report_descriptor = {
    0x05, 0x01, /* USAGE_PAGE (Generic Desktop) */
    0x09, 0x06, /* USAGE (Keyboard) */
    0xa1, 0x01, /* COLLECTION (Application) */
    0x05, 0x07, /*   USAGE_PAGE (Keyboard) */
    0x19, 0xe0, /*   USAGE_MINIMUM (224) */
    0x29, 0xe7, /*   USAGE_MAXIMUM (231) */
    0x15, 0x00, /*   LOGICAL_MINIMUM (0) */
    0x25, 0x01, /*   LOGICAL_MAXIMUM (1) */
    0x75, 0x01, /*   REPORT_SIZE (1) */
    0x95, 0x08, /*   REPORT_COUNT (8) */
    0x81, 0x02, /*   INPUT (Data,Var,Abs); Modifier byte */
    0x75, 0x08, /*   REPORT_SIZE (8) */
    0x95, 0x01, /*   REPORT_COUNT (1) */
    0x81, 0x01, /*   INPUT (Constant); Reserved byte */
    0x75, 0x01, /*   REPORT_SIZE (1) */
    0x95, 0x05, /*   REPORT_COUNT (5) */
    0x05, 0x08, /*   USAGE_PAGE (LEDs) */
    0x19, 0x01, /*   USAGE_MINIMUM (1) */
    0x29, 0x05, /*   USAGE_MAXIMUM (5) */
    0x91, 0x02, /*   OUTPUT (Data,Var,Abs); LED report */
    0x75, 0x03, /*   REPORT_SIZE (3) */
    0x95, 0x01, /*   REPORT_COUNT (1) */
    0x91, 0x01, /*   OUTPUT (Constant); LED report padding */
    0x75, 0x08, /*   REPORT_SIZE (8) */
    0x95, 0x06, /*   REPORT_COUNT (6) */
    0x15, 0x00, /*   LOGICAL_MINIMUM (0) */
    0x25, 0x65, /*   LOGICAL_MAXIMUM (101) */
    0x05, 0x07, /*   USAGE_PAGE (Keyboard) */
    0x19, 0x00, /*   USAGE_MINIMUM (0) */
    0x29, 0x65, /*   USAGE_MAXIMUM (101) */
    0x81, 0x00, /*   INPUT (Data,Ary,Abs) */
    0xC0,       /* END_COLLECTION  */
};

UsbHidDevice::HidDescriptor hid_descriptor = {
    .bLength = sizeof(UsbHidDevice::HidDescriptor),
    .bDescriptorType = UsbHidDevice::kDescriptorTypeHid,
    .bcdHID = 0x0110,
    .bCountryCode = 0x00,
    .bNumDescriptors = 1,
    .bReportDescriptorType = UsbHidDevice::kDescriptorTypeReport,
    .wDescriptorLength = static_cast<uint16_t>(report_descriptor.size())};

UsbDevice::ConfigurationDescriptor configuration_descriptor = {
    .bLength = sizeof(configuration_descriptor),
    .bDescriptorType = UsbDevice::kDescriptorTypeConfiguration,
    .wTotalLength = static_cast<uint16_t>(
        sizeof(UsbDevice::ConfigurationDescriptor) +
        sizeof(UsbDevice::InterfaceDescriptor) +
        sizeof(UsbDevice::EndPointDescriptor) * endpoint_descriptors.size() +
        sizeof(UsbHidDevice::HidDescriptor)),
    .bNumInterfaces = static_cast<uint8_t>(interface_descriptors.size()),
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xc0,
    .bMaxPower = 250};

std::vector<std::string> strings = {"", "", ""};

}  // namespace

UsbHidKeyboard::UsbHidKeyboard(uint16_t vendor_id,
                               uint16_t product_id,
                               uint16_t version,
                               std::string vendor_name,
                               std::string product_name,
                               std::string version_name)
    : UsbHidDevice(device_descriptor,
                   configuration_descriptor,
                   interface_descriptors,
                   endpoint_descriptors,
                   hid_descriptor,
                   report_descriptor,
                   strings) {
  device_descriptor.idVendor = vendor_id;
  device_descriptor.idProduct = product_id;
  device_descriptor.bcdDevice = version;
  strings[kManufacturerStringIndex - 1] = vendor_name;
  strings[kProductStringIndex - 1] = product_name;
  strings[kSerialNumberStringIndex - 1] = version_name;

  report_.resize(8);
}

void UsbHidKeyboard::SetAutoKeyRelease(bool enabled) {
  auto_key_release_ = enabled;
}

void UsbHidKeyboard::SetModifiers(uint8_t modifiers) {
  modifiers_ = modifiers;
}

void UsbHidKeyboard::PressByUsageId(uint8_t usage_id) {
  keycodes_.insert(usage_id);
  if (reporting_) {
    dirty_ = true;
  } else {
    Report();
  }
}

void UsbHidKeyboard::ReleaseByUsageId(uint8_t usage_id) {
  auto it = keycodes_.find(usage_id);
  if (it != keycodes_.end()) {
    keycodes_.erase(it);
    if (reporting_) {
      dirty_ = true;
    } else {
      Report();
    }
  }
}

void UsbHidKeyboard::OnCompleteToSend(uint8_t endpoint) {
  reporting_ = false;
  if (dirty_) {
    Report();
  }
}

void UsbHidKeyboard::Report() {
  report_[0] = modifiers_;
  report_[1] = 0x00;
  if (keycodes_.size() > 6) {
    // Phantom state
    for (size_t i = 2; i < report_.size(); ++i) {
      report_[i] = 0x01;
    }
  } else {
    size_t i = 2;
    for (size_t keycode : keycodes_) {
      report_[i++] = keycode;
    }
    while (i < 8) {
      report_[i++] = 0x00;
    }
  }
  dirty_ = false;
  if (auto_key_release_ && !keycodes_.empty()) {
    keycodes_.clear();
    modifiers_ = 0;
    dirty_ = true;
  }
  reporting_ = true;
  UsbHidDevice::Report(1, report_);
}