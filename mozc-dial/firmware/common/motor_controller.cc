// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include "motor_controller.h"

#include <cstdio>
#include <initializer_list>

#include "hardware/gpio.h"
#include "pico/time.h"

static constexpr int kMotorStepIntervalInUs = 2000;

// Activate 1 of 8 motor drivers in a specific step phase.
class MotorController::Decoder {
 public:
  Decoder(int8_t a0, int8_t a1, int8_t a2, int8_t en)
      : a0_(a0), a1_(a1), a2_(a2), en_(en) {
    for (int8_t gpio : {en, a0, a1, a2}) {
      gpio_init(gpio);
      gpio_set_dir(gpio, GPIO_OUT);
      gpio_put(gpio, false);
    }
  }
  Decoder(const Decoder&) = delete;
  Decoder& operator=(const Decoder&) = delete;
  ~Decoder() = default;

  void Select(uint8_t index) {
    hard_assert(index < 8);
    gpio_put(a0_, index & 1);
    gpio_put(a1_, index & 2);
    gpio_put(a2_, index & 4);
    gpio_put(en_, true);
  }
  void Unselect() { gpio_put(en_, false); }

 private:
  int8_t a0_;
  int8_t a1_;
  int8_t a2_;
  int8_t en_;
};

// static
bool MotorController::OnAlarm(repeating_timer* t) {
  static_cast<MotorController*>(t->user_data)->Step();
  return true;
}

MotorController::MotorController(Mode mode) {
  if (mode == Mode::k9Motor) {
    // Each decoder is responsible for a specific phase to driver 1 of 8 motor
    // drivers. As we have 4 phases, it can drive 4 motors at the same time, and
    // by time division multiplexing, we make them drive 8 motors.
    decoder_[0] = std::make_unique<Decoder>(1, 2, 3, 4);
    decoder_[1] = std::make_unique<Decoder>(5, 6, 7, 8);
    decoder_[2] = std::make_unique<Decoder>(9, 10, 11, 12);
    decoder_[3] = std::make_unique<Decoder>(13, 14, 15, 16);
  }

  // 9th motor driver is controlled via GPIOs directly.
  for (int8_t gpio : {17, 18, 19, 20}) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, false);
  }

  add_repeating_timer_us(-kMotorStepIntervalInUs, OnAlarm, this, &timer_);
}

MotorController::~MotorController() {}

void MotorController::Start(uint8_t index) {
  hard_assert(index < kNumOfMotors);
  started_[index] = true;
}

void MotorController::Stop(uint8_t index) {
  hard_assert(index < kNumOfMotors);
  started_[index] = false;
}

void MotorController::Step() {
  // Drive first 4 motor drivers in even phases, and later 4 ones in odd
  // phases.
  size_t start_index = (phase_ & 1) ? kNumOfPhases : 0;
  size_t half_phase = phase_ >> 1;
  for (size_t i = start_index; i < start_index + kNumOfPhases; ++i) {
    if (!decoder_[i]) {
      continue;
    }
    if (started_[i]) {
      decoder_[(i + half_phase) % kNumOfPhases]->Select(i);
    } else {
      decoder_[(i + half_phase) % kNumOfPhases]->Unselect();
    }
  }
  // Drive the last 9th motor drivers.
  bool on = started_[8] && phase_ & 1;
  gpio_put(17, on && half_phase == 0);
  gpio_put(18, on && half_phase == 1);
  gpio_put(19, on && half_phase == 2);
  gpio_put(20, on && half_phase == 3);

  phase_ = (phase_ + 7) % 8;
}