#pragma once

#include "base/action.h"
#include "base/world_ext.h"
#include "solvers/solver.h"
#include "common/unsigned_set.h"
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

  WorldExt world;
  std::vector<POI> poi;
  UnsignedSet poi_assigned;

  std::vector<std::pair<unsigned, unsigned>> target;  // Size-Representative
  UnsignedSet acw1, acw2;

 protected:
  void Init(const std::string& task);
  void CleanPOI();
  void UpdateTarget();
  bool AssignClosestWorker(unsigned r, ActionsList& al);
  ActionType SendToNearestCodeX(unsigned windex);
  ActionType SendToNearestUnwrapped(unsigned windex);
  void NextMove_Init(ActionsList& al);
  void NextMove_Wrap(ActionsList& al);
  ActionsList NextMove();

 public:
  ActionsClones Solve(const std::string& task, unsigned _strategy,
                      const std::string& bonuses);
};
}  // namespace solvers
