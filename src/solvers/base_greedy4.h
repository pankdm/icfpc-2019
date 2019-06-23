#pragma once

#include "solvers/base_greedy_old3.h"
#include <set>

namespace solvers {

class BaseGreedy4 : public BaseGreedyOld3 {
  std::set<std::pair<int, int>> bonuses;
  Action NextMove();
  int mops_to_go;

 public:
  BaseGreedy4(BaseGreedy3Settings);
  ActionsList Solve(const std::string& task);
};

}  // namespace solvers
