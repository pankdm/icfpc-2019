#pragma once

#include <cstdint>

enum class Item : uint8_t {
  NONE = 0,
  EXTENSION,
  FAST_WHEELS,
  DRILL,
  TELEPORT,
  BEACON,
  CODEX,
  UNKNOWN
};

const unsigned TIME_FAST_WHEELS = 50;
const unsigned TIME_DRILL = 30;
