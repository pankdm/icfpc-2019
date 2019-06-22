#pragma once

#include "base/world.h"
#include "solvers/solver.h"
#include "common/graph/graph.h"
#include "common/unsigned_set.h"
#include <vector>

namespace solvers {
class BaseGreedy2 : public Solver {
 protected:
  World world;

  UndirectedGraph g;
  UnsignedSet unwrapped;

 protected:
  void Init(const std::string& task);
  Action NextMove();
  void Update();
  bool Wrapped();

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
