#pragma once

#include "solvers/base_greedy2.h"
#include <set>

namespace solvers {
class BaseGreedy3 : public BaseGreedy2 {
  std::set<std::pair<int, int>> bonuses;
  Action NextMove();

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
