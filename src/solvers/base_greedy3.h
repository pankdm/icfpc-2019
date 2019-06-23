#pragma once

#include "solvers/base_greedy_old2.h"
#include "solvers/settings/base_greedy3.h"
#include <set>

namespace solvers {
class BaseGreedy3 : public BaseGreedyOld2 {
  std::set<std::pair<int, int>> bonuses;
  Action NextMove();

 protected:
  int mops_to_go;
  BaseGreedy3Settings settings;

 public:
  BaseGreedy3(BaseGreedy3Settings);
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
