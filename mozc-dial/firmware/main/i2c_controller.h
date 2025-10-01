// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#ifndef I2C_CONTROLLER_H_
#define I2C_CONTROLLER_H_

#include <cstdint>
#include <span>

#include "hardware/i2c.h"

class I2CController final {
 public:
  I2CController(i2c_inst_t* i2c, int8_t sda, int8_t scl);
  I2CController(const I2CController&) = delete;
  I2CController& operator=(const I2CController&) = delete;
  ~I2CController() = default;

  bool Write(uint8_t device_addr, uint8_t mem_addr, std::span<uint8_t> data);
  bool Read(uint8_t device_addr, uint8_t mem_addr, std::span<uint8_t> data);

 private:
  i2c_inst_t* i2c_ = nullptr;
};

#endif  // I2C_CONTROLLER_H_