#include "solvers/worker/local2.h"

#include "base/point.h"
#include "solvers/paths/path_to_target.h"
#include "common/always_assert.h"
#include <algorithm>
#include <unordered_map>

namespace solvers {
namespace worker {
void Local2::UpdateTarget() {
  thread_local std::unordered_map<unsigned, unsigned> rep_to_size;
  thread_local std::unordered_map<unsigned, unsigned> rep_to_score;
  thread_local std::queue<std::pair<unsigned, unsigned>> q;
  thread_local UnsignedSet s;
  rep_to_size.clear();
  rep_to_score.clear();
  for (; !q.empty();) q.pop();
  if (s.TotalSize() < pworld->Size()) {
    s.Resize(pworld->Size());
  }
  s.Clear();

  target.Clear();
  auto w = Get();
  const auto& task = pworld->GetTask(tindex);
  if (task.Empty()) return;
  for (unsigned u : task.List()) {
    rep_to_size[pworld->DSFind(u)] = pworld->DSSize(u);
  }

  // score = dist + size
  unsigned best_score = unsigned(-1);
  const unsigned DIST_WEIGHT = 2;
  q.push(std::make_pair(pworld->Index(w.x, w.y), 0));
  s.Insert(pworld->Index(w.x, w.y));
  for (; !q.empty(); q.pop()) {
    unsigned index = q.front().first;
    unsigned dist = q.front().second;
    if (task.HasKey(index)) {
      unsigned r = pworld->DSFind(index);
      if (rep_to_score.find(r) == rep_to_score.end()) {
        unsigned score = dist * DIST_WEIGHT + rep_to_size[r];
        rep_to_score[r] = score;
        if (score < best_score) {
          best_score = score;
        }
      }
    }
    if (dist * DIST_WEIGHT >= best_score) break;
    for (unsigned u : pworld->GEdges(index)) {
      if (!s.HasKey(u)) {
        q.push(std::make_pair(u, dist + 1));
        s.Insert(u);
      }
    }
  }
  for (unsigned u : task.List()) {
    auto r = pworld->DSFind(u);
    if (rep_to_score.find(r) != rep_to_score.end()) {
      if (rep_to_score[r] == best_score) target.Insert(u);
    }
  }
  ALWAYS_ASSERT(!target.Empty());
}

void Local2::Init(unsigned _windex, unsigned _tindex, WorldTaskSplit& world) {
  Base::Init(_windex, _tindex, world);
  target.Resize(world.Size());
  UpdateTarget();
}

void Local2::ResetTask(unsigned new_index) {
  Base::ResetTask(new_index);
  UpdateTarget();
}

Action Local2::NextMove() {
  ALWAYS_ASSERT(pworld);
  const auto& task = pworld->GetTask(tindex);
  if (task.Empty()) return ActionType::DO_NOTHING;
  if (pworld->UpdateTaskRequired(tindex)) {
    pworld->UpdateTask(tindex);
    UpdateTarget();
  }
  if (pworld->boosters.extensions.Available({pworld->time, windex})) {
    auto p = Get().GetNextManipulatorPositionNaive(0);
    pworld->boosters.extensions.LockUntilPicked();
    Action a(ActionType::ATTACH_MANIPULATOR, p.first, p.second);
    return a;
  }

  Action a = PathToTarget(Base::Get(), *pworld, target);

  bool need_turn = true;
  Direction d(a.type);
  Direction wd = Get().direction;
  Point pw(Get().x, Get().y);
  Point next = pw + d;
  for (int i = 0; i < 4; i++) {
    if (!pworld->map.Inside(next.x, next.y) ||
        pworld->map.Get(next.x, next.y).WrappedOrBlocked()) {
      need_turn = false;
    }
    next = next + d;
  }
  if (need_turn) {
    if (d.direction == wd.direction - 1) {
      return Action(ActionType::ROTATE_CLOCKWISE);
    } else {
      return Action(ActionType::ROTATE_COUNTERCLOCKWISE);
    }
  }
  return a;
}
}  // namespace worker
}  // namespace solvers
