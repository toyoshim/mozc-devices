// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#ifndef COMMON_DIAL_CONTROLLER_H
#define COMMON_DIAL_CONTROLLER_H

#include <cstdint>
#include <optional>

class DialController {
 public:
  DialController();
  DialController(const DialController&) = delete;
  DialController& operator=(const DialController&) = delete;
  ~DialController() = default;

  void Update(uint8_t sensor_gray_code);
  bool IsBasePosition() const;
  std::optional<uint8_t> PopDecidedPosition();

 private:
  uint8_t position_ = 0u;
  uint8_t max_position_ = 0u;
  std::optional<uint8_t> decided_position_;
};

#endif  // COMMON_DIAL_CONTROLLER_H