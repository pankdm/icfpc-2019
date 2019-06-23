#pragma once

#include "base/action.h"
#include "base/world.h"
#include "solvers/solver.h"
#include "common/disjoint_set.h"
#include "common/graph/graph.h"
#include "common/unsigned_set.h"
#include <utility>
#include <vector>

namespace solvers {
class ClonesGreedy : public Solver {
 public:
  class POI {
   public:
    Item item;
    int index;
    std::vector<unsigned> vd;
  };

 protected:
  unsigned strategy;

  World world;
  UndirectedGraph g;
  UnsignedSet unwrapped;
  std::vector<POI> poi;
  UnsignedSet poi_assigned;

  DisjointSet ds;
  std::vector<std::pair<unsigned, unsigned>> target;  // Size-Representative
  UnsignedSet ds_rebuid_required;
  UnsignedSet ds_rebuid;
  UnsignedSet acw1, acw2;

 protected:
  void Init(const std::string& task);
  void CleanPOI();
  void BuildDSUnsignedSet();
  void BuildDS();
  void RebuildDS();
  void UpdateTarget();
  bool AssignClosestWorker(unsigned r, ActionsList& al);
  ActionType SendToNearestCodeX(unsigned windex);
  ActionType SendToNearestUnwrapped(unsigned windex);
  void NextMove_Init(ActionsList& al);
  void NextMove_Wrap(ActionsList& al);
  ActionsList NextMove();
  void Update();
  bool Wrapped();

 public:
  ActionsClones Solve(const std::string& task, unsigned _strategy);
};
}  // namespace solvers
