#pragma once

#include "solvers/solver.h"

namespace solvers {
class Auto : public Solver {
 public:
  ActionsClones Solve(const std::string& task, const std::string& task_name,
                      const std::string& bonuses);
};
}  // namespace solvers
