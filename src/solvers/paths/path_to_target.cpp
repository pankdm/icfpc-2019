#include "solvers/paths/path_to_target.h"

#include "base/direction.h"
#include "base/point.h"
#include "common/always_assert.h"
#include "common/unsigned_set.h"
#include <queue>

namespace solvers {
Action PathToTarget(const Worker& w, const WorldExt& world,
                    const UnsignedSet& target) {
  thread_local UnsignedSet visited;
  thread_local std::queue<std::pair<unsigned, Action>> q;
  if (visited.Size() < world.Size()) visited.Resize(world.Size());
  visited.Clear();
  for (; !q.empty();) q.pop();

  unsigned wi = world.Index(w.x, w.y);
  Point pw(w.x, w.y);
  if (target.HasKey(wi)) return Action(ActionType::DO_NOTHING);
  for (unsigned _d = 0; _d < 4; ++_d) {
    Direction d(_d);
    Point pd = pw + d;
    if (world.map.ValidToMove(pd.x, pd.y)) {
      unsigned inext = world.Index(pd.x, pd.y);
      q.push(std::make_pair(inext, Action(d.Get())));
      visited.Insert(inext);
    }
  }
  // TODO:
  //   Add beacons check.
  for (; !q.empty(); q.pop()) {
    unsigned index = q.front().first;
    if (target.HasKey(index)) return q.front().second;
    for (unsigned inext : world.GEdges(index)) {
      if (!visited.HasKey(inext)) {
        q.push(std::make_pair(inext, q.front().second));
        visited.Insert(inext);
      }
    }
  }
  ALWAYS_ASSERT(false);
  return Action(ActionType::DO_NOTHING);
}
}  // namespace solvers
