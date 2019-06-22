#pragma once

#include "base/item.h"
#include <assert.h>
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

  void SetBlock() { value |= mask_blocked; }

  void Drill() { value &= ~mask_blocked; }

  void Wrap() {
    if (!Blocked()) value |= mask_wrapped;
  }

  Item CheckItem() const { return Item(value & mask); }

  Item GetItem() {
    Item t = CheckItem();
    if ((t != Item::BEACON) && (t != Item::CODEX)) value &= ~mask;
    return t;
  }

  void AddItem(Item item) {
    assert(value == 0);
    value = uint8_t(item);
  }

  void SetBeacon() {
    assert((value & mask) == 0);
    value += uint8_t(Item::BEACON);
  }
};
