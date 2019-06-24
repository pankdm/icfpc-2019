#pragma once

#include "base/action.h"
#include "base/point.h"
#include "base/world.h"
#include "solvers/solver.h"
#include "common/disjoint_set.h"
#include "common/graph/graph.h"
#include "common/unsigned_set.h"
#include <utility>
#include <vector>

namespace solvers {
class BaseClones1Settings {
 public:
  size_t manip_index;
  size_t strategy;
  size_t ext_dist;
  bool use_shifts;
  bool even_mops;
  bool use_teleports;
};

class BaseClones1 : public Solver {
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
  UnsignedSet poi_assigned;

  bool reset_beacon{};
  Point beacon{-1, -1};
  std::vector<int> beaconDist;

  DisjointSet ds;
  std::vector<std::pair<unsigned, unsigned>> target;  // Size-Representative
  UnsignedSet ds_rebuid_required;
  UnsignedSet ds_rebuid;
  UnsignedSet acw1, acw2;
  BaseClones1Settings sett;

 protected:
  void Init(const std::string& task, BaseClones1Settings sett);
  void CleanPOI();
  void BuildDSUnsignedSet();
  void BuildDS();
  void RebuildDS();
  void UpdateTarget();
  bool AssignClosestWorker(unsigned r, ActionsList& al);
  Action SendToNearestUnwrapped(unsigned windex);
  void NextMove_Clone(ActionsList& al);
  void NextMove_Wrap(ActionsList& al);
  bool NextMove_SetBeacon(unsigned windex, Action& action);
  bool NextMove_Shift(unsigned windex, unsigned dest_index, unsigned now_distance, Action& action);
  ActionsList NextMove();
  void Update();
  bool Wrapped();

 public:
  ActionsClones Solve(const std::string& task, BaseClones1Settings sett,
                      const std::string& bonuses);
};
}  // namespace solvers
