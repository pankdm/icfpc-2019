#pragma once

#include "base/world.h"
#include "solvers/solver.h"
#include "common/graph/graph.h"
#include <vector>

namespace solvers {
class BaseGreedy2 : public Solver {
 protected:
  World world;

  std::vector<int> unwrapped;
  UndirectedGraph g;

 protected:
  void Init(const std::string& task);
  Action NextMove();

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
