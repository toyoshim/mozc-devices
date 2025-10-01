// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#ifndef COMMON_USAGE_TABLES_
#define COMMON_USAGE_TABLES_

#include <vector>
#include <cstdint>

namespace usage_tables {

static constexpr uint8_t kModifierLeftCtrl = (1 << 0);
static constexpr uint8_t kModifierLeftShift = (1 << 1);
static constexpr uint8_t kModifierLeftAlt = (1 << 2);
static constexpr uint8_t kModifierLeftGUI = (1 << 3);
static constexpr uint8_t kModifierRightCtrl = (1 << 4);
static constexpr uint8_t kModifierRightShift = (1 << 5);
static constexpr uint8_t kModifierRightAlt = (1 << 6);
static constexpr uint8_t kModifierRightGUI = (1 << 7);

extern const std::vector<uint8_t> a;
extern const std::vector<uint8_t> b;
extern const std::vector<uint8_t> c;
extern const std::vector<uint8_t> d;
extern const std::vector<uint8_t> e;
extern const std::vector<uint8_t> f;
extern const std::vector<uint8_t> g;
extern const std::vector<uint8_t> h;
extern const std::vector<uint8_t> i;

extern const std::vector<uint8_t> fn_a;
extern const std::vector<uint8_t> fn_b;
extern const std::vector<uint8_t> fn_c;
extern const std::vector<uint8_t> fn_d;
extern const std::vector<uint8_t> fn_e;
extern const std::vector<uint8_t> fn_f;
extern const std::vector<uint8_t> fn_g;
extern const std::vector<uint8_t> fn_h;
extern const std::vector<uint8_t> fn_i;

extern const std::vector<uint8_t> modifier_a;
extern const std::vector<uint8_t> modifier_b;
extern const std::vector<uint8_t> modifier_c;
extern const std::vector<uint8_t> modifier_d;
extern const std::vector<uint8_t> modifier_e;
extern const std::vector<uint8_t> modifier_f;
extern const std::vector<uint8_t> modifier_g;
extern const std::vector<uint8_t> modifier_h;
extern const std::vector<uint8_t> modifier_i;

}  // namespace usage_tables

#endif  // COMMON_USAGE_TABLES_