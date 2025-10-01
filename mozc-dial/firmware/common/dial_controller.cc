// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include "dial_controller.h"

#include <algorithm>

namespace {

constexpr uint8_t kBasePosition = 0;

uint8_t ConvertGrayToBinary(uint8_t gray) {
  uint8_t binary = gray;
  uint8_t mask = gray >> 1;

  while (mask) {
    binary ^= mask;
    mask >>= 1;
  }
  return binary;
}

}  // namespace

DialController::DialController() {}

void DialController::Update(uint8_t sensor_gray_code) {
  position_ = ConvertGrayToBinary(sensor_gray_code);
  max_position_ = std::max(max_position_, position_);
  if (position_ == kBasePosition && max_position_ != kBasePosition) {
    decided_position_ = max_position_;
    max_position_ = kBasePosition;
  }
}

bool DialController::IsBasePosition() const {
  return position_ == kBasePosition;
}

std::optional<uint8_t> DialController::PopDecidedPosition() {
  auto result = decided_position_;
  decided_position_ = std::nullopt;
  return result;
}