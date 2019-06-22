#pragma once

#include "solvers/base_greedy2.h"

namespace solvers {
class BaseGreedy3 : public BaseGreedy2 {
  Action NextMove();

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
