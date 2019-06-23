#pragma once

#include "base/world.h"
#include "solvers/solver.h"
#include "common/disjoint_set.h"
#include "common/graph/graph.h"
#include "common/unsigned_set.h"
#include <vector>

namespace solvers {
class BaseGreedy2 : public Solver {
 protected:
  World world;

  UndirectedGraph g;
  UnsignedSet unwrapped;
  DisjointSet ds;
  UnsignedSet target;
  UnsignedSet ds_rebuid_required;
  UnsignedSet ds_rebuid;

 protected:
  void Init(const std::string& task);
  void BuildDSUnsignedSet();
  void BuildDS();
  void RebuildDS();
  void SetTarget(unsigned dist_weight = 1);
  Action NextMove();
  void Update();
  bool Wrapped();

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solvers
