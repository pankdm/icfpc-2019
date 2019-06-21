#pragma once

#include "base/action_type.h"
#include <vector>

class Action {
 public:
  ActionType type;
  int8_t x, y;
};

using ActionsList = std::vector<Action>;
