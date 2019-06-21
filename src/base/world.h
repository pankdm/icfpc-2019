#pragma once

#include "base/action.h"
#include "base/map.h"
#include "base/worker.h"

class World {
 public:
  Map map;
  Worker worker;

 public:
  void Apply(const Action& action) { worker.Apply(map, action); }
  void Apply(const ActionsList& actions) {
    for (const Action& action : actions) Apply(action);
  }
};
