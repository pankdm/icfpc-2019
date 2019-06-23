#pragma once

#include "base/world_ext.h"
#include "solvers/solver.h"
#include "common/unsigned_set.h"
#include <vector>

namespace solvers {
class BaseGreedy2 : public Solver {
 protected:
  WorldExt world;
  UnsignedSet target;

 protected:
  void Init(const std::string& task);
  void SetTarget(unsigned dist_weight = 1);
  Action NextMove();

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
