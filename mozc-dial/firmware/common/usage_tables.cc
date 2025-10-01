// Copyright 2025 Google Inc.
// Use of this source code is governed by an Apache License that can be found
// in the LICENSE file.

#include "usage_tables.h"

namespace usage_tables {

const std::vector<uint8_t> a = {
    0x00,  // no input (blank area between the base and the first position)
    0x2f,  // @
    0x34,  // :
    0x87,  // _
    0x13,  // p
    0x33,  // ;
    0x38,  // /
    0x12,  // o
    0x0f,  // l
    0x37,  // .
    0x0c,  // i
    0x0e,  // k
    0x36,  // ,
    0x18,  // u
    0x0d,  // j
    0x10,  // m
    0x1c,  // y
    0x0b,  // h
    0x11,  // n
    0x17,  // t
    0x0a,  // g
    0x05,  // b
    0x15,  // r
    0x09,  // f
    0x19,  // v
    0x08,  // e
    0x07,  // d
    0x06,  // c
    0x1a,  // w
    0x16,  // s
    0x1b,  // x
    0x14,  // q
    0x04,  // a
    0x1d,  // z
    0x39,  // <CAPS>
    0x2c,  // <SPACE>
};
const std::vector<uint8_t> fn_a = {};
const std::vector<uint8_t> modifier_a = {};

const std::vector<uint8_t> b = {
    0x2e,  // ^ (~)
    0x29,  // <ESC>
    0x2b,  // <TAB>
};
const std::vector<uint8_t> fn_b = {};
const std::vector<uint8_t> modifier_b = {};

const std::vector<uint8_t> c = {
    0x00,  // <SHIFT>
    0x00,  // <CTRL>
    0x00,  // <ALT>
    0x00,  // <FN>
};
const std::vector<uint8_t> fn_c = {};
const std::vector<uint8_t> modifier_c = {
    kModifierLeftShift,
    kModifierLeftCtrl,
    kModifierLeftAlt,
    kModifierLeftGUI,
};

const std::vector<uint8_t> d = {
    0x00,  // no input
    0x28,  // <ENTER>
};
const std::vector<uint8_t> fn_d = {};
const std::vector<uint8_t> modifier_d = {};

const std::vector<uint8_t> e = {
    0x00,  // no input
    0x4d,  // <END>
    0x4e,  // <PAGE DOWN>
    0x4b,  // <PAGE UP>
    0x4a,  // <HOME>
    0x49,  // <INS>
    0x4c,  // <DEL>
};
const std::vector<uint8_t> fn_e = {};
const std::vector<uint8_t> modifier_e = {};

const std::vector<uint8_t> f = {
    0x00,  // no input
    0x4f,  // <RIGHT>
    0x52,  // <UP>
    0x50,  // <LEFT>
    0x51,  // <DOWN>
};
const std::vector<uint8_t> fn_f = {};
const std::vector<uint8_t> modifier_f = {};

const std::vector<uint8_t> g = {
    0x55,  // KP-*
    0x56,  // KP-/
    0x63,  // KP-.
};
const std::vector<uint8_t> fn_g = {};
const std::vector<uint8_t> modifier_g = {};

const std::vector<uint8_t> h = {
    0x57,  // KP-+
    0x56,  // KP--
    0x67,  // KP-=
};
const std::vector<uint8_t> fn_h = {};
const std::vector<uint8_t> modifier_h = {};

const std::vector<uint8_t> i = {
    0x00,  // no input
    0x26,  // 9
    0x25,  // 8
    0x24,  // 7
    0x23,  // 6
    0x22,  // 5
    0x21,  // 4
    0x20,  // 3
    0x1f,  // 2
    0x1e,  // 1
    0x27,  // 0
};
const std::vector<uint8_t> fn_i = {
    0x00,  // no input
    0x42,  // F9
    0x41,  // F8
    0x40,  // F7
    0x3f,  // F6
    0x3e,  // F5
    0x3d,  // F4
    0x3c,  // F3
    0x3b,  // F2
    0x3a,  // F1
    0x43,  // F10
};
const std::vector<uint8_t> modifier_i = {};

}  // namespace usage_tables