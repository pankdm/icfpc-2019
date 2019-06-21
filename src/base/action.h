#pragma once

#include "base/action_type.h"
#include <vector>

class Action {
 public:
  ActionType type;
  int16_t x, y;
};

using ActionsList = std::vector<Action>;
