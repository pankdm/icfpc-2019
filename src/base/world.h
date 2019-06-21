#pragma once

#include "base/action.h"
#include "base/map.h"
#include "base/worker.h"

class World {
 public:
  Map map;
  Worker worker;
  unsigned time = 0;

 public:
  void Apply(const Action& action);
  void Apply(const ActionsList& actions);
  bool Solved() const;
};
