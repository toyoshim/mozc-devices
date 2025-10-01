// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#ifndef I2C_DEVICE_H_
#define I2C_DEVICE_H_

#include <cstdint>
#include <functional>

#include "hardware/i2c.h"
#include "pico/i2c_slave.h"

class I2CDevice final {
 public:
  using ReadHandler = std::function<uint8_t(uint8_t /*address*/)>;
  using WriteHandler = std::function<void(uint8_t /*address*/, uint8_t /*value*/)>;

  I2CDevice(i2c_inst_t* i2c, uint8_t sda, uint8_t scl, uint8_t address);
  I2CDevice(const I2CDevice&) = delete;
  I2CDevice& operator=(const I2CDevice&) = delete;
  ~I2CDevice() = default;

  void SetReadHandler(ReadHandler handler);
  void SetWriteHandler(WriteHandler handler);

 private:
  static void HandleEvent(i2c_inst_t* i2c, i2c_slave_event_t event);
  void HandleEventInternal(i2c_inst_t* i2c, i2c_slave_event_t event);

  ReadHandler reader_ = nullptr;
  WriteHandler writer_ = nullptr;
  bool address_ready_ = false;
  uint8_t address_ = 0;
};

#endif  // I2C_DEVICE_H_