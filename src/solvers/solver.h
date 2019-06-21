#pragma once

#include "base/action.h"
#include <string>

namespace solvers {
class Solver {
 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
