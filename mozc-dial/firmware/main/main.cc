// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include <array>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "hardware/uart.h"
#if LIB_PICO_STDIO_UART
#include "pico/stdio_uart.h"
#endif
#include "pico/stdlib.h"

#include "../common/dial_controller.h"
#include "../common/motor_controller.h"
#include "../common/photo_sensor.h"
#include "../common/usage_tables.h"
#include "../common/usb_hid_keyboard.h"
#include "i2c_controller.h"

int main() {
#if LIB_PICO_STDIO_UART
  stdio_uart_init_full(uart0, /*baud_rate=*/115200, /*tx_pin=*/0,
                       /*rx_pin=*/-1);
#else
  stdio_init_all();
#endif

  I2CController i2c(i2c0, /*sda=*/28, /*scl=*/29);

  // Sensor A, B, C, D, E, F, G, I (note: H is missing here as it's accessed via
  // I2C)
  std::array<PhotoSensor, 8> sensors = {
      PhotoSensor(1, 2, 3, 4, 5, 6),  // A
      PhotoSensor(7, 8),              // B
      PhotoSensor(9, 10, 11),         // C
      PhotoSensor(12, 13),            // D (3 sensors are implemented, but...)
      PhotoSensor(15, 16, 17),        // E
      PhotoSensor(18, 19, 20),        // F
      PhotoSensor(21, 22),            // G (3 sensors are implemented, but...)
      PhotoSensor(24, 25, 26, 27),    // I
  };

  std::array<DialController, 9> dials = {
      DialController(),  // A
      DialController(),  // B
      DialController(),  // C
      DialController(),  // D
      DialController(),  // E
      DialController(),  // F
      DialController(),  // G
      DialController(),  // I

      DialController(),  // H
  };

  std::array<const std::vector<uint8_t>*, 9> usages = {
      &usage_tables::a,  // A
      &usage_tables::b,  // B
      &usage_tables::c,  // C
      &usage_tables::d,  // D
      &usage_tables::e,  // E
      &usage_tables::f,  // F
      &usage_tables::g,  // G
      &usage_tables::i,  // I

      &usage_tables::h,  // H
  };

  std::array<const std::vector<uint8_t>*, 9> fn_usages = {
      &usage_tables::fn_a,  // A
      &usage_tables::fn_b,  // B
      &usage_tables::fn_c,  // C
      &usage_tables::fn_d,  // D
      &usage_tables::fn_e,  // E
      &usage_tables::fn_f,  // F
      &usage_tables::fn_g,  // G
      &usage_tables::fn_i,  // I

      &usage_tables::fn_h,  // H
  };

  std::array<const std::vector<uint8_t>*, 9> modifiers = {
      &usage_tables::modifier_a,  // A
      &usage_tables::modifier_b,  // B
      &usage_tables::modifier_c,  // C
      &usage_tables::modifier_d,  // D
      &usage_tables::modifier_e,  // E
      &usage_tables::modifier_f,  // F
      &usage_tables::modifier_g,  // G
      &usage_tables::modifier_i,  // I

      &usage_tables::modifier_h,  // H
  };

  UsbHidKeyboard usb_hid_keyboard(
      /*vendor_id=*/0x6666, /*product_id=*/0x2025,
      /*version=*/0x0109, /*vendor_name=*/"Gboard DIY prototype",
      /*product_name=*/"Gboard Dial version", /*version_name=*/"9 Dial");
  usb_hid_keyboard.SetAutoKeyRelease(true);

  // Wait for a while, just in case, so that the sub-controller can be ready on
  // I2C.
  sleep_ms(100);

  std::vector<uint8_t> i2c_buffer(2);
  bool fn = false;

  while (true) {
    uint16_t motor_start_bitmap = 0;
    for (size_t i = 0; i < sensors.size(); ++i) {
      dials[i].Update(sensors[i].Read());
      if (!dials[i].IsBasePosition()) {
        motor_start_bitmap |= (1 << i);
      }
    }
    // Read the remote sensor H value via I2C.
    bool rc = i2c.Read(68, 0, std::span<uint8_t>({i2c_buffer.data(), 1}));
    if (rc) {
      dials[8].Update(i2c_buffer[0]);
      if (!dials[8].IsBasePosition()) {
        motor_start_bitmap |= (1 << 8);
      }
    }

    // Drive all motors via I2C.
    i2c_buffer[0] = motor_start_bitmap & 0xff;
    i2c_buffer[1] = motor_start_bitmap >> 8;
    rc = i2c.Write(68, 0, i2c_buffer);

    // Check all dials to see if we have pending inputs to send over USB HID.
    for (size_t i = 0; i < dials.size(); ++i) {
      // `decided_position` is std::nullopt while the dial is not moved at the
      // base position, or moving. But once it returns to the base position from
      // non-base positions, it will have a 1 or a larger value that indicates
      // the position where the dial starts returning.
      std::optional<uint8_t> decided_position = dials[i].PopDecidedPosition();
      if (!decided_position) {
        continue;
      }
      // One-time flip to use an alternative set of usages, by the Fn key.
      std::array<const std::vector<uint8_t>*, 9>& current_usages =
          fn ? fn_usages : usages;

      // Adjust the index as it is 1-based.
      size_t index = *decided_position - 1;

      if (index < current_usages[i]->size()) {
        uint8_t usage = (*current_usages[i])[index];
        if (usage) {
          usb_hid_keyboard.PressByUsageId(usage);
          fn = false;
        }
      }
      if (index < modifiers[i]->size()) {
        uint8_t modifier = (*modifiers[i])[index];
        if (modifier == usage_tables::kModifierLeftGUI) {
          // This device interprets the left GUI key as a one-time keymap
          // alternate key instead of the normal GUI modifier key use.
          fn = true;
        } else {
          usb_hid_keyboard.SetModifiers(
              (usb_hid_keyboard.GetModifiers() | modifier));
        }
      }
    }
  }
}