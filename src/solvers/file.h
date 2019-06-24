#pragma once

#include "solvers/solver.h"

namespace solvers {
class File : public Solver {
 public:
  ActionsClones Solve(const std::string& task, const std::string& task_name,
                      std::string filename = "");
};
}  // namespace solvers
