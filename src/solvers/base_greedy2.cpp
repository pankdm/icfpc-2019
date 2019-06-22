#include "solvers/base_greedy2.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/point.h"
#include "base/worker.h"
#include <cassert>
#include <queue>
#include <unordered_set>
#include <utility>

namespace solvers {
void BaseGreedy2::Init(const std::string& task) {
  world.Init(task);
  unsigned size = unsigned(world.map.Size());
  auto& map = world.map;
  g.Resize(size);
  for (int x = 0; x < map.xsize; ++x) {
    for (int y = 0; y < map.ysize; ++y) {
      if (map.ValidToMove(x, y)) {
        int index = map.Index(x, y);
        if (map.ValidToMove(x + 1, y)) g.AddEdge(index, map.Index(x + 1, y));
        if (map.ValidToMove(x, y + 1)) g.AddEdge(index, map.Index(x, y + 1));
      }
    }
  }
}

Action BaseGreedy2::NextMove() {
  thread_local std::unordered_set<int> s;
  thread_local std::queue<std::pair<int, Direction>> q;
  s.clear();
  for (; !q.empty();) q.pop();
  Point pw(world.worker.x, world.worker.y);
  for (unsigned _d = 0; _d < 4; ++_d) {
    Direction d(_d);
    Point pd = pw + d;
    if (world.map.ValidToMove(pd.x, pd.y)) {
      int index = world.map.Index(pd.x, pd.y);
      q.push(std::make_pair(index, d));
      s.insert(index);
    }
  }
  for (; !q.empty(); q.pop()) {
    int index = q.front().first;
    Direction d = q.front().second;
    if (!world.map[index].Wrapped()) {
      Action a(d.Get());
      world.Apply(a);
      return a;
    }
    for (int inext : g.Edges(index)) {
      if (s.find(inext) == s.end()) {
        q.push(std::make_pair(inext, d));
        s.insert(inext);
      }
    }
  }
  assert(false);
  return Action(ActionType::END);
}  // namespace solvers

ActionsList BaseGreedy2::Solve(const std::string& task) {
  Init(task);
  ActionsList actions;
  for (; !world.Solved();) {
    Action a = NextMove();
    if (a.type == ActionType::END) return actions;
    actions.emplace_back(a);
  }
  return actions;
}
}  // namespace solvers
