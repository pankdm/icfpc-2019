#pragma once

#include "base/action_type.h"
#include <cstdint>
#include <vector>

class Action {
 public:
  ActionType type;
  int16_t x, y;

 public:
  Action();
  Action(ActionType _type);
  Action(ActionType _type, int _x, int _y);
};

using ActionsList = std::vector<Action>;
using ActionsClones = std::vector<ActionsList>;
