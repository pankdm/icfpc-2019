#pragma once

#include "base/action_type.h"

class Direction {
 public:
  unsigned direction;

 public:
  Direction(unsigned d);
  Direction(ActionType t);

  int DX() const;
  int DY() const;
  ActionType Get() const;
  void Set(ActionType t);
};
