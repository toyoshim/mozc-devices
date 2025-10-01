// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include <cstdio>

#include "hardware/uart.h"
#include "pico/stdio.h"
#if LIB_PICO_STDIO_UART
#include "pico/stdio_uart.h"
#endif

#include "../common/motor_controller.h"
#include "../common/photo_sensor.h"
#include "i2c_device.h"

namespace {

MotorController motor_controller;
PhotoSensor sensor_h(26, 27);

uint8_t i2c_reader(uint8_t address) {
  switch (address) {
    case 0:  // Read Sensor H
      return sensor_h.Read();
    default:
      return 0xff;
  }
}

void i2c_writer(uint8_t address, uint8_t value) {
  switch (address) {
    case 0:  // Set Motor 1-8 State
      for (uint8_t motor = 0; motor < 8; ++motor) {
        if (value & (1 << motor)) {
          motor_controller.Start(motor);
        } else {
          motor_controller.Stop(motor);
        }
      }
      break;
    case 1:  // Set Motor 9 State
      if (value & 1) {
        motor_controller.Start(8);
      } else {
        motor_controller.Stop(8);
      }
      break;
  }
}

}  // namespace

int main() {
#if LIB_PICO_STDIO_UART
  // Customized initialization to use only GPIO 0 for TX.
  stdio_uart_init_full(uart0, /*baud_rate=*/115200, /*tx_pin=*/0,
                       /*rx_pin=*/-1);
#else
  stdio_init_all();
#endif

  I2CDevice i2c(/*i2c=*/i2c0, /*sda=*/28, /*scl=*/29, /*address=*/68);
  i2c.SetReadHandler(i2c_reader);
  i2c.SetWriteHandler(i2c_writer);

  while (true) {
    tight_loop_contents();
  }
}
