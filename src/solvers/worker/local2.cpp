#include "solvers/worker/local2.h"

#include "base/point.h"
#include "solvers/paths/path_to_target.h"
#include "common/always_assert.h"
#include <algorithm>

namespace solvers {
namespace worker {
void Local2::UpdateTarget() {
  target.Clear();
  auto w = Get();
  // score = dist + size
  int best_score = -1;
  std::map<int, int> rep_to_score;
  std::map<int, int> rep_to_size;
  for (unsigned u : pworld->UList()) {
    rep_to_size[pworld->DSFind(u)] = pworld->DSSize(u);
  }
  std::queue<std::pair<int, int>> q;
  UnsignedSet s(pworld->map.Size());
  q.push(std::make_pair(pworld->map.Index(w.x, w.y), 0));
  s.Insert(pworld->map.Index(w.x, w.y));
  for (; !q.empty(); q.pop()) {
    int index = q.front().first;
    int dist = q.front().second;
    Point px(pworld->map.X(index), pworld->map.Y(index));
    if (!pworld->map.Get(px.x, px.y).WrappedOrBlocked()) {
      int DIST_WEIGHT = 2;
      if (best_score != -1 && DIST_WEIGHT * dist > best_score) {
        break;
      }
      int rep = pworld->DSFind(index);
      if (rep_to_score.count(rep) == 0) {
        int score = dist * DIST_WEIGHT + pworld->DSSize(rep);
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
      if (!pworld->map.Inside(p.x, p.y)) {
        continue;
      }
      int index = pworld->map.Index(p.x, p.y);
      if (s.HasKey(index)) {
        continue;
      }
      q.push(std::make_pair(index, dist + 1));
      s.Insert(index);
    }
  }
  for (unsigned u : pworld->UList()) {
    if (rep_to_score[pworld->DSFind(u)] == best_score) {
      target.Insert(u);
    }
  }
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
  return PathToTarget(Base::Get(), *pworld, target);
}
}  // namespace worker
}  // namespace solvers
