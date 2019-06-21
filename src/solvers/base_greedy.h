#pragma once

#include "base/world.h"
#include "solvers/solver.h"

namespace solvers {
class BaseGreedy : public Solver {
 protected:
  World world;

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
