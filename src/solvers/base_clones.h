#pragma once

#include "base/action.h"
#include "base/world.h"
#include "solvers/solver.h"
#include "common/graph/graph.h"
#include "common/unsigned_set.h"
#include <vector>

namespace solvers {
class BaseClones : public Solver {
 public:
  class POI {
   public:
    Item item;
    int index;
    std::vector<unsigned> vd;
  };

 protected:
  World world;
  UndirectedGraph g;
  UnsignedSet unwrapped;
  std::vector<POI> poi;

 protected:
  void Init(const std::string& task);
  void CleanPOI();
  ActionsList NextMove();

 public:
  ActionsClones Solve(const std::string& task);
};
}  // namespace solvers
