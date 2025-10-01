// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#ifndef COMMON_PHOTO_SENSOR_H_
#define COMMON_PHOTO_SENSOR_H_

#include <array>
#include <cstddef>
#include <cstdint>

class PhotoSensor final {
 public:
  PhotoSensor(int8_t bit0,
              int8_t bit1 = kNotUsed,
              int8_t bit2 = kNotUsed,
              int8_t bit3 = kNotUsed,
              int8_t bit4 = kNotUsed,
              int8_t bit5 = kNotUsed);
  PhotoSensor(const PhotoSensor&) = delete;
  PhotoSensor& operator=(const PhotoSensor&) = delete;
  ~PhotoSensor() = default;

  uint8_t Read();

 private:
  static constexpr int8_t kNotUsed = -1;
  static constexpr size_t kMaxWidth = 6;

  std::array<int8_t, kMaxWidth> bits;
};

#endif  // COMMON_PHOTO_SENSOR_H_