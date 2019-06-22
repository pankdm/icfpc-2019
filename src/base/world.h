#pragma once

#include "base/action.h"
#include "base/boosters.h"
#include "base/map.h"
#include "base/worker.h"
#include <string>

class World {
 public:
  Map map;
  Boosters boosters;
  Worker worker;
  unsigned time = 0;

 public:
  void Init(const std::string& desc);
  void Apply(const Action& action);
  void Apply(const ActionsList& actions);
  bool Solved() const;
};
