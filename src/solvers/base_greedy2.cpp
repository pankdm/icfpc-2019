#include "solvers/base_greedy2.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/point.h"
#include "base/worker.h"
#include "common/always_assert.h"
#include "common/unsigned_set.h"
#include "common/vector/write.h"
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
  ds_rebuid_required.Resize(size);
  ds_rebuid.Resize(size);
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
  BuildDS();
}

void BaseGreedy2::BuildDSUnsignedSet() {
  for (unsigned u : ds_rebuid.List()) {
    for (unsigned t : g.Edges(u)) {
      if ((t > u) && unwrapped.HasKey(t)) {
        ds.Union(u, t);
      }
    }
  }
}

void BaseGreedy2::BuildDS() {
  ds.Init(world.map.Size());
  ds_rebuid = unwrapped;
  BuildDSUnsignedSet();
  SetTarget();
}

void BaseGreedy2::RebuildDS() {
  ds_rebuid.Clear();
  for (unsigned u : unwrapped.List()) {
    if (ds_rebuid_required.HasKey(ds.Find(u))) ds_rebuid.Insert(u);
  }
  ds.unions -= (ds_rebuid.Size() - ds_rebuid_required.Size());
  for (unsigned u : ds_rebuid.List()) {
    ds.p[u] = u;
    ds.rank[u] = 0;
    ds.vsize[u] = 1;
  }
  BuildDSUnsignedSet();
}

void BaseGreedy2::SetTarget(unsigned dist_weight) {
  if (dist_weight == 0) {
    unsigned min_ds_size = ds.Size();
    for (unsigned u : unwrapped.List()) {
      min_ds_size = std::min(min_ds_size, ds.GetSize(u));
    }
    target.Clear();
    for (unsigned u : unwrapped.List()) {
      if (ds.GetSize(u) == min_ds_size) target.Insert(u);
    }
  } else {
    target.Clear();
    auto w = world.GetWorker();
    // score = dist + size
    int best_score = -1;
    std::map<int, int> rep_to_score;
    std::map<int, int> rep_to_size;
    for (unsigned u : unwrapped.List()) {
      rep_to_size[ds.Find(u)] = ds.GetSize(u);
    }
    std::queue<std::pair<int, int>> q;
    UnsignedSet s(world.map.Size());
    q.push(std::make_pair(world.map.Index(w.x, w.y), 0));
    s.Insert(world.map.Index(w.x, w.y));
    for (; !q.empty(); q.pop()) {
      int index = q.front().first;
      int dist = q.front().second;
      Point px(world.map.X(index), world.map.Y(index));
      if (!world.map.Get(px.x, px.y).WrappedOrBlocked()) {
        if (best_score != -1 && dist_weight * dist > best_score) {
          break;
        }
        int rep = ds.Find(index);
        if (rep_to_score.count(rep) == 0) {
          int score = dist * dist_weight + ds.GetSize(rep);
          rep_to_score[rep] = score;
          if (best_score == -1 || score < best_score) {
            best_score = score;
          }
        }
        if (rep_to_score.size() == rep_to_size.size()) {
          break;
        }
      }
      for (int i = 0; i < 4; i++) {
        Point p = px + Direction(i);
        if (!world.map.Inside(p.x, p.y)) {
          continue;
        }
        int index = world.map.Index(p.x, p.y);
        if (s.HasKey(index)) {
          continue;
        }
        q.push(std::make_pair(index, dist + 1));
        s.Insert(index);
      }
    }
    for (unsigned u : unwrapped.List()) {
      if (rep_to_score[ds.Find(u)] == best_score) {
        target.Insert(u);
      }
    }
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
  if (!ds_rebuid_required.Empty()) {
    RebuildDS();
    ds_rebuid_required.Clear();
    SetTarget();
  }
  Point pw(world.GetWorker().x, world.GetWorker().y);
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
    int index = q.front();
    if (unwrapped.HasKey(index)) {
      unwrapped.Remove(index);
      ds_rebuid_required.Insert(ds.Find(index));
    }
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
