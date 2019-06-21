#include "solutions/base_greedy.h"

#include "base/direction.h"
#include "base/point.h"
#include "base/worker.h"
#include <cassert>
#include <queue>
#include <utility>

namespace solutions {
ActionsList BaseGreedy::Solve(const std::string& task) {
  world.Init(task);
  ActionsList actions;
  for (; !world.Solved();) {
    Point p(world.worker.x, world.worker.y);
    std::queue<std::pair<Point, Direction>> q;
    for (unsigned _d = 0; _d < 4; ++_d) {
      Direction d(_d);
      Point pd = p + d;
      if (world.map.ValidToMove(pd.x, pd.y)) q.push(std::make_pair(pd, d));
    }
    for (; !q.empty(); q.pop()) {
      Point p = q.front().first;
      Direction d = q.front().second;
      if (!world.map(p.x, p.y).Wrapped()) {
        Action a(d.Get());
        actions.push_back(a);
        world.Apply(a);
        break;
      }
      for (unsigned _d = 0; _d < 4; ++_d) {
        Point pd = p + Direction(_d);
        if (world.map.ValidToMove(pd.x, pd.y)) q.push(std::make_pair(pd, d));
      }
    }
    if (q.empty()) {
      assert(false);
      break;
    }
  }
  return actions;
}
}  // namespace solutions
