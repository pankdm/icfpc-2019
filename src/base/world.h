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
  void Apply(const Action& action) { worker.Apply(++time, map, action); }

  void Apply(const ActionsList& actions) {
    for (const Action& action : actions) Apply(action);
  }

  bool Solved() const { return map.Wrapped(); }
};
