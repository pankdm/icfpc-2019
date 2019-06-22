#include "solvers/base_greedy3.h"
#include "base/action_type.h"
#include "base/point.h"

namespace solvers {

Action BaseGreedy3::NextMove() {
  if (world.boosters.unused_extensions) {
    auto p = world.GetWorker().GetNextManipulatorPositionNaive();
    Action a(ActionType::ATTACH_MANIPULATOR, p.first, p.second);
    world.Apply(a);
    Update();
    return a;
  }
  auto p = std::make_pair(world.GetWorker().x, world.GetWorker().y);
  if (bonuses.count(p) != 0) {
    bonuses.erase(p);
  }
  for (auto p : world.GetWorker().CellsToWrap(world.map)) {
    if (world.map.HasExtension(p.first, p.second)) {
      bonuses.insert(p);
    }
  }
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
    if (target.HasKey(index) || world.map.HasExtension(index)) {
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

ActionsList BaseGreedy3::Solve(const std::string& task) {
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
