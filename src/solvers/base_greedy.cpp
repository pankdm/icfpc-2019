#include "solvers/base_greedy.h"

#include "base/direction.h"
#include "base/point.h"
#include "base/worker.h"
#include "common/always_assert.h"
#include <queue>
#include <unordered_set>
#include <utility>

namespace solvers {
ActionsList BaseGreedy::Solve(const std::string& task) {
  world.Init(task);
  ActionsList actions;
  for (; !world.Solved();) {
    std::unordered_set<int> s;
    Point pw(world.GetWorker().x, world.GetWorker().y);
    std::queue<std::pair<Point, Direction>> q;
    for (unsigned _d = 0; _d < 4; ++_d) {
      Direction d(_d);
      Point pd = pw + d;
      if (world.map.ValidToMove(pd.x, pd.y)) {
        q.push(std::make_pair(pd, d));
        s.insert((pd.x << 16) + pd.y);
      }
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
        int hkey = (pd.x << 16) + pd.y;
        if (world.map.ValidToMove(pd.x, pd.y) && (s.find(hkey) == s.end())) {
          q.push(std::make_pair(pd, d));
          s.insert(hkey);
        }
      }
    }
    if (q.empty()) {
      ALWAYS_ASSERT(false);
      break;
    }
  }
  return actions;
}
}  // namespace solvers
