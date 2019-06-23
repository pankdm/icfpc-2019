#pragma once

#include "base/action.h"
#include <string>

namespace solvers {
class Solver {
 public:
  ActionsList Solve(const std::string& task);
  ActionsList Solve(const std::string& task, const std::string& bonuses);
};
}  // namespace solvers
