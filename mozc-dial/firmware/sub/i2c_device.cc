// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include "i2c_device.h"

#include <cstdio>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"

static constexpr int kI2CSpeedInHz = 400 * 1000;

namespace {
I2CDevice* sI2CDevice = nullptr;
};  // namespace

// static
void I2CDevice::HandleEvent(i2c_inst_t* i2c, i2c_slave_event_t event) {
  sI2CDevice->HandleEventInternal(i2c, event);
}

I2CDevice::I2CDevice(i2c_inst_t* i2c,
                     uint8_t sda,
                     uint8_t scl,
                     uint8_t address) {
  hard_assert(sI2CDevice == nullptr);
  sI2CDevice = this;

  gpio_init(sda);
  gpio_init(scl);
  gpio_set_function(sda, GPIO_FUNC_I2C);
  gpio_set_function(scl, GPIO_FUNC_I2C);
  gpio_pull_up(sda);
  gpio_pull_up(scl);

  i2c_init(i2c, kI2CSpeedInHz);
  i2c_slave_init(i2c, address, &I2CDevice::HandleEvent);
}

void I2CDevice::SetReadHandler(ReadHandler handler) {
  reader_ = handler;
}

void I2CDevice::SetWriteHandler(WriteHandler handler) {
  writer_ = handler;
}

void I2CDevice::HandleEventInternal(i2c_inst_t* i2c, i2c_slave_event_t event) {
  switch (event) {
    case I2C_SLAVE_RECEIVE:
      if (!address_ready_) {
        address_ = i2c_read_byte_raw(i2c);
        address_ready_ = true;
      } else {
        if (writer_) {
          writer_(address_, i2c_read_byte_raw(i2c));
        } else {
          printf("ignore write at $%02x: $%02x\n", address_, i2c_read_byte_raw(i2c));
        }
        ++address_;
      }
      break;
    case I2C_SLAVE_REQUEST:
      i2c_write_byte_raw(i2c, reader_ ? reader_(address_) : 0);
      address_++;
      break;
    case I2C_SLAVE_FINISH:
      address_ready_ = false;
      break;
  }
}
