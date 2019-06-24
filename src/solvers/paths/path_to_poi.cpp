#include "solvers/paths/path_to_poi.h"

#include "base/direction.h"
#include "base/point.h"
#include "common/always_assert.h"

namespace solvers {
Action PathToPOI(const Worker& w, const WorldExt& world, const POI& poi) {
  unsigned windex = world.Index(w.x, w.y);
  if (poi.location == windex) return ActionType::DO_NOTHING;
  unsigned distance = poi.vd[windex];
  ALWAYS_ASSERT((distance > 0) && (distance < unsigned(-1)));
  const auto& map = world.map;
  Point pw(w.x, w.y);
  for (unsigned _d = 0; _d < 4; ++_d) {
    Direction d(_d);
    Point pd = pw + d;
    if (world.map.ValidToMove(pd.x, pd.y)) {
      unsigned pdi = world.Index(pd.x, pd.y);
      if (distance == poi.vd[pdi] + 1) return d.Get();
    }
  }
  ALWAYS_ASSERT(false);
  return ActionType::DO_NOTHING;
}

Action PathToPOI(const Worker& w, const WorldExt& world, const POIList& list) {
  ALWAYS_ASSERT(list.items.size() > 0);
  unsigned windex = world.Index(w.x, w.y);
  unsigned best_distance = unsigned(-1);
  unsigned closest_poi = 0;
  for (unsigned i = 0; i < list.items.size(); ++i) {
    if (list.items[i].vd[windex] < best_distance) {
      best_distance = list.items[i].vd[windex];
      closest_poi = i;
    }
  }
  return PathToPOI(w, world, list.items[closest_poi]);
}
}  // namespace solvers
