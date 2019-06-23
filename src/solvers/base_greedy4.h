#pragma once

#include "solvers/base_greedy3.h"
#include <set>

namespace solvers {

class BaseGreedy4 : public BaseGreedy3 {
  std::set<std::pair<int, int>> bonuses;
  Action NextMove();
  int mops_to_go;

 public:
  BaseGreedy4(BaseGreedy3Settings);
  ActionsList Solve(const std::string& task);
};

}  // namespace solvers
