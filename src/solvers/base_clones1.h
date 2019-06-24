#pragma once

#include "base/action.h"
#include "base/world_ext.h"
#include "solvers/settings/base_clones1.h"
#include "solvers/solver.h"
#include "common/unsigned_set.h"
#include <utility>
#include <vector>

namespace solvers {
class BaseClones1 : public Solver {
 public:
  class POI {
   public:
    Item item;
    int index;
    std::vector<unsigned> vd;
  };

 protected:
  WorldExt world;
  std::vector<POI> poi;
  UnsignedSet poi_assigned;

  std::vector<std::pair<unsigned, unsigned>> target;  // Size-Representative
  UnsignedSet acw1, acw2;
  BaseClones1Settings sett;

 protected:
  void Init(const std::string& task, BaseClones1Settings sett);
  void CleanPOI();
  void UpdateTarget();
  bool AssignClosestWorker(unsigned r, ActionsList& al);
  Action SendToNearestUnwrapped(unsigned windex);
  void NextMove_Clone(ActionsList& al);
  void NextMove_Wrap(ActionsList& al);
  ActionsList NextMove();

 public:
  ActionsClones Solve(const std::string& task, BaseClones1Settings sett,
                      const std::string& bonuses);
};
}  // namespace solvers
