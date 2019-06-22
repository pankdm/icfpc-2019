#include "solvers/base_greedy2.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/point.h"
#include "base/worker.h"
#include "common/unsigned_set.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <queue>
#include <utility>

namespace solvers {
void BaseGreedy2::Init(const std::string& task) {
  world.Init(task);
  unsigned size = unsigned(world.map.Size());
  auto& map = world.map;
  map.save_wraps = true;
  g.Resize(size);
  unwrapped.Clear();
  unwrapped.Resize(size);
  target.Resize(size);
  for (int x = 0; x < map.xsize; ++x) {
    for (int y = 0; y < map.ysize; ++y) {
      if (map.ValidToMove(x, y)) {
        int index = map.Index(x, y);
        if (map.ValidToMove(x + 1, y)) g.AddEdge(index, map.Index(x + 1, y));
        if (map.ValidToMove(x, y + 1)) g.AddEdge(index, map.Index(x, y + 1));
        if (!map[index].Wrapped()) unwrapped.Insert(index);
      }
    }
  }
}

void BaseGreedy2::BuildDS(const std::vector<unsigned>& v) {
  ds.Init(world.map.Size());
  for (unsigned u : v) {
    for (unsigned t : g.Edges(u)) {
      if ((t > u) && unwrapped.HasKey(t)) {
        ds.Union(u, t);
      }
    }
  }
}

void BaseGreedy2::SetTarget() {
  unsigned min_ds_size = ds.Size();
  for (unsigned u : unwrapped.List()) {
    min_ds_size = std::min(min_ds_size, ds.GetSize(u));
  }
  target.Clear();
  for (unsigned u : unwrapped.List()) {
    if (ds.GetSize(u) == min_ds_size) target.Insert(u);
  }
}

Action BaseGreedy2::NextMove() {
  thread_local UnsignedSet s;
  thread_local std::queue<std::pair<int, Direction>> q;
  if (s.Size() < world.map.Size()) {
    s.Resize(world.map.Size());
  } else {
    s.Clear();
  }
  for (; !q.empty();) q.pop();
  BuildDS(unwrapped.List());
  SetTarget();
  Point pw(world.worker.x, world.worker.y);
  for (unsigned _d = 0; _d < 4; ++_d) {
    Direction d(_d);
    Point pd = pw + d;
    if (world.map.ValidToMove(pd.x, pd.y)) {
      int index = world.map.Index(pd.x, pd.y);
      q.push(std::make_pair(index, d));
      s.Insert(index);
    }
  }
  for (; !q.empty(); q.pop()) {
    int index = q.front().first;
    Direction d = q.front().second;
    if (target.HasKey(index)) {
      Action a(d.Get());
      world.Apply(a);
      Update();
      return a;
    }
    for (int inext : g.Edges(index)) {
      if (!s.HasKey(inext)) {
        q.push(std::make_pair(inext, d));
        s.Insert(inext);
      }
    }
  }
  std::cout << std::endl;
  world.map.Print();
  assert(false);
  return Action(ActionType::END);
}

void BaseGreedy2::Update() {
  auto& q = world.map.wraps_history;
  for (; !q.empty(); q.pop()) {
    unwrapped.Remove(q.front());
  }
}

bool BaseGreedy2::Wrapped() { return unwrapped.Empty(); }

ActionsList BaseGreedy2::Solve(const std::string& task) {
  Init(task);
  ActionsList actions;
  for (; !Wrapped();) {
    Action a = NextMove();
    if (a.type == ActionType::END) return actions;
    actions.emplace_back(a);
  }
  return actions;
}
}  // namespace solvers
