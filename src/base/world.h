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
  std::vector<Worker> workers;
  unsigned time = 0;

 public:
  void Init(const std::string& desc);
  Worker& GetWorker(unsigned index = 0);
  void Apply(const Action& action);
  void Apply(const ActionsList& actions);
  bool Solved() const;

  // Clone versions
  unsigned WCount() const;
  void ApplyC(unsigned index, const Action& action);
  void ApplyC(const ActionsList& actions);
  void ApplyC(const ActionsClones& actions);
};
