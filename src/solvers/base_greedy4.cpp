#include "solvers/base_greedy4.h"
#include "base/action_type.h"
#include "base/point.h"

namespace solvers {

BaseGreedy4::BaseGreedy4(BaseGreedy3Settings _settings)
    : BaseGreedy3(_settings) {}

Action BaseGreedy4::NextMove() {
  if (world.boosters.unused_extensions) {
    auto p =
        world.GetWorker().GetNextManipulatorPositionNaive(settings.use_sword);
    if (mops_to_go == 1 || (mops_to_go > 0 && world.map.items_coords.count(
                                                  Item::EXTENSION) == 0)) {
      BuildDS();
      ds_rebuid_required.Clear();
      SetTarget();
    }
    mops_to_go--;

    Action a(ActionType::ATTACH_MANIPULATOR, p.first, p.second);
    world.Apply(a);
    Update();
    return a;
  }

  thread_local UnsignedSet s;
  thread_local std::queue<std::pair<int, Direction>> q;
  if (s.Size() < world.map.Size()) {
    s.Resize(world.map.Size());
  } else {
    s.Clear();
  }
  Point pw(world.GetWorker().x, world.GetWorker().y);
  for (; !q.empty();) q.pop();
  if (!ds_rebuid_required.Empty()) {
    RebuildDS();
    ds_rebuid_required.Clear();
    SetTarget();
  }

  if (mops_to_go > 0 && world.map.items_coords.count(Item::EXTENSION) > 0) {
    target.Clear();
    for (auto p : world.map.items_coords[Item::EXTENSION]) {
      target.Insert(world.map.Index(p.first, p.second));
    }
  }

  int best_sz = -1;
  int d_best = -1;
  for (unsigned _d = 0; _d < 4; ++_d) {
    Direction d((_d + world.GetWorker().direction.direction) % 4);
    Point pd = pw + d;
    if (world.map.ValidToMove(pd.x, pd.y)) {
      int index = world.map.Index(pd.x, pd.y);
      int new_sz =
          world.GetWorker().CellsToNewlyWrap(world.map, d.DX(), d.DY()).size();
      if (new_sz > best_sz) {
        d_best = d.direction;
        best_sz = new_sz;
      }
    }
  }
  for (int i = 0; i < 4; i++) {
    Direction d((d_best + i) % 4);
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
      // first turn
      if (d.direction % 2 != world.GetWorker().direction.direction % 2) {
        bool need_turn = true;
        Point next = pw + d;
        for (int i = 0; i < 2; i++) {
          if (world.map.Inside(next.x, next.y) &&
              world.map.Get(next.x, next.y).WrappedOrBlocked()) {
            need_turn = false;
          }
          next = next + d;
        }
        if (need_turn) {
          if (d.direction == world.GetWorker().direction.direction - 1) {
            Action a(ActionType::ROTATE_CLOCKWISE);
            world.Apply(a);
            Update();
            return a;
          } else {
            Action a(ActionType::ROTATE_COUNTERCLOCKWISE);
            world.Apply(a);
            Update();
            return a;
          }
        }
      }
      if (settings.use_shifts) {
        for (int i = 0; i < 2; i++) {
          Direction d((1 + 2 * i + world.GetWorker().direction.direction) % 4);
          int score = 0;
          Point base = pw;
          Direction wd = world.GetWorker().direction;
          for (int iter = 0; iter <= 3; iter++) {
            if (iter > 0) {
              base = base + wd;
            }
            if (!world.map.ValidToMove(base.x, base.y)) {
              continue;
            }
            if (iter > 1 && world.map.Get(base.x, base.y).Wrapped()) {
              continue;
            }
            int sz =
                world.GetWorker()
                    .CellsToNewlyWrap(world.map, wd.DX() * iter, wd.DY() * iter)
                    .size();
            Point pd = base + d;
            if (!world.map.ValidToMove(pd.x, pd.y)) {
              continue;
            }
            int new_sz =
                world.GetWorker()
                    .CellsToNewlyWrap(world.map, wd.DX() * iter + d.DX(),
                                      wd.DY() * iter + d.DY())
                    .size();
            if (new_sz > sz) {
              score++;
            }
          }
          if (score == 4) {
            Action a(d.Get());
            world.Apply(a);
            Update();
            return a;
          }
        }
      }
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
}  // namespace solvers

ActionsList BaseGreedy4::Solve(const std::string& task) {
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
