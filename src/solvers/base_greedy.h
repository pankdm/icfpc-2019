#pragma once

#include "base/world_ext.h"
#include "solvers/solver.h"

namespace solvers {
class BaseGreedy : public Solver {
 protected:
  WorldExt world;

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
