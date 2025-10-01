// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include <cstdint>
#include <cstdio>
#include <optional>

#include "pico/stdlib.h"

#include "../common/dial_controller.h"
#include "../common/motor_controller.h"
#include "../common/photo_sensor.h"
#include "../common/usage_tables.h"
#include "../common/usb_hid_keyboard.h"

int main() {
  // GPIO0 and 1 are used for stdout, and stdin by default.
  stdio_init_all();

  // GPIO17, 18, 19, and 20 are used for motor phase control.
  MotorController motor_controller(MotorController::Mode::k1Motor);

  // GPIO2, 3, 4, 5, 6, and 7 are used for 6bit photo sensing.
  PhotoSensor photo_sensor(2, 3, 4, 5, 6, 7);

  DialController dial_controller;

  UsbHidKeyboard usb_hid_keyboard(
      /*vendor_id=*/0x6666, /*product_id=*/0x2025,
      /*version=*/0x0101, /*vendor_name=*/"Gboard DIY prototype",
      /*product_name=*/"Gboard Dial version", /*version_name=*/"1 Dial");
  usb_hid_keyboard.SetAutoKeyRelease(true);

  while (true) {
    dial_controller.Update(photo_sensor.Read());
    if (dial_controller.IsBasePosition()) {
      motor_controller.Stop(8);
    } else {
      motor_controller.Start(8);
    }
    // `decided_position` is std::nullopt while the dial is not moved at the
    // base position, or moving. But once it returns to the base position from
    // non-base positions, it will have a 1 or a larger value that indicates the
    // position where the dial starts returning.
    std::optional<uint8_t> decided_position =
        dial_controller.PopDecidedPosition();
    if (decided_position && *decided_position <= usage_tables::a.size()) {
      // Adjust the index as it is 1-based.
      uint8_t usage = usage_tables::a[*decided_position - 1];
      if (usage) {
        usb_hid_keyboard.PressByUsageId(usage);
      }
    }
  }
}
