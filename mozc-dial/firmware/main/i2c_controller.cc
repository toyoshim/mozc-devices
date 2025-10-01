// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include "i2c_controller.h"

#include <algorithm>
#include <vector>

#include "hardware/gpio.h"

static constexpr int kI2CSpeedInHz = 400 * 1000;

I2CController::I2CController(i2c_inst_t* i2c, int8_t sda, int8_t scl)
    : i2c_(i2c) {
  hard_assert(i2c == i2c0 || i2c == i2c1);
  gpio_init(sda);
  gpio_init(scl);
  gpio_set_function(sda, GPIO_FUNC_I2C);
  gpio_set_function(scl, GPIO_FUNC_I2C);
  gpio_pull_up(sda);
  gpio_pull_up(scl);
  i2c_init(i2c_, kI2CSpeedInHz);
}

bool I2CController::Read(uint8_t device_addr,
                         uint8_t mem_addr,
                         std::span<uint8_t> data) {
  uint8_t addr[1] = {mem_addr};
  int result = i2c_write_blocking(i2c_, device_addr, addr, 1, true);
  if (result != 1) {
    return false;
  }
  result = i2c_read_blocking(i2c_, device_addr, data.data(), data.size(), false);
  return result == data.size();
}

bool I2CController::Write(uint8_t device_addr,
                          uint8_t mem_addr,
                          std::span<uint8_t> data) {
  std::vector<uint8_t> buf;
  buf.reserve(data.size() + 1);
  buf.push_back(mem_addr);
  buf.insert(buf.end(), data.begin(), data.end());
  int rc = i2c_write_blocking(i2c_, device_addr, buf.data(), buf.size(), false);
  return rc == buf.size();
}