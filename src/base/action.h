#pragma once

#include "base/action_type.h"
#include <vector>

class Action {
 public:
  ActionType type;
};

using ActionsList = std::vector<Action>;
