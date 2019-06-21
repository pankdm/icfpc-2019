#pragma once

#include <cstdint>

class Square {
 public:
  uint8_t value = 0;

  static const uint8_t mask = 63;
  static const uint8_t mask_blocked = 128;
  static const uint8_t mask_wrapped = 64;

  bool Blocked() const { return value & mask_blocked; }
  bool Wrapped() const { return value & mask_wrapped; }
  bool WrappedOrBlocked() const {
    return value & (mask_blocked | mask_wrapped);
  }

  void Wrap() {
    if (!Blocked()) value |= mask_wrapped;
  }

  uint8_t Item() const { return value & mask; }
};
