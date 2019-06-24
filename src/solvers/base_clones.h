#pragma once

#include "base/action.h"
#include "base/world_ext.h"
#include "solvers/solver.h"
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
  void NextMove_Clone(ActionsList& al);
  void NextMove_Wrap(ActionsList& al);
  ActionsList NextMove();

 public:
  ActionsClones Solve(const std::string& task, const std::string& bonuses);
};
}  // namespace solvers
