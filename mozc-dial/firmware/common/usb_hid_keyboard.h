// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#ifndef COMMON_USB_HID_KEYBOARD_H_
#define COMMON_USB_HID_KEYBOARD_H_

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "usb_hid_device.h"

class UsbHidKeyboard : public UsbHidDevice {
 public:
  UsbHidKeyboard(uint16_t vendor_id,
                 uint16_t product_id,
                 uint16_t version,
                 std::string vendor_name,
                 std::string product_name,
                 std::string version_name);
  UsbHidKeyboard(const UsbHidKeyboard&) = delete;
  UsbHidKeyboard& operator=(const UsbHidKeyboard&) = delete;
  ~UsbHidKeyboard() override = default;

  void SetAutoKeyRelease(bool enabled);

  void SetModifiers(uint8_t modifiers);
  uint8_t GetModifiers() const { return modifiers_; }

  void PressByUsageId(uint8_t usage_id);
  void ReleaseByUsageId(uint8_t usage_id);

 private:
  // Implement UsbDevice.
  void OnCompleteToSend(uint8_t endpoint) override;

  void Report();

  bool auto_key_release_ = false;
  bool reporting_ = false;
  bool dirty_ = false;
  uint8_t modifiers_ = 0;
  std::set<uint8_t> keycodes_;
  std::vector<uint8_t> report_;
};

#endif  // COMMON_USB_HID_KEYBOARD_H_
