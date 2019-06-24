#include "solvers/worker/local2.h"

#include "base/point.h"
#include "solvers/paths/path_to_poi.h"
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
      // int MOP_DIST = 5;
      // if (dist < MOP_DIST &&
      //     pworld->map[index].CheckItem() == Item::EXTENSION) {
      //   target.Insert(index);
      //   return;
      // }
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
  if (a.type == ActionType::DO_NOTHING) {
    return a;
  }

  unsigned min_index = unsigned(-1), max_index = 0;
  for (unsigned u : task.List()) {
    min_index = std::min(u, min_index);
    max_index = std::max(u, max_index);
  }
  unsigned best_distance_to_poi = unsigned(-1);
  unsigned poi_index = 0;
  for (unsigned i = 0; i < pworld->GetPOIList(Item::EXTENSION).items.size();
       ++i) {
    auto& poi = pworld->GetPOIList(Item::EXTENSION).items[i];
    if ((poi.location >= min_index) && (poi.location <= max_index)) {
      auto w = Get();
      unsigned windex = pworld->Index(w.x, w.y);
      if (poi.vd[windex] < best_distance_to_poi) {
        best_distance_to_poi = poi.vd[windex];
        poi_index = i;
      }
    }
  }
  if (best_distance_to_poi < (unsigned(-1))) {
    return PathToPOI(Get(), *pworld,
                     pworld->GetPOIList(Item::EXTENSION).items[poi_index]);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  for (int i = 0; i < 2; i++) {
    auto w = Get();
    auto pw = Point(w.x, w.y);
    Direction d((1 + 2 * i + w.direction.direction) % 4);
    int score = 0;
    Direction wd = w.direction;
    int MAX = 4;
    bool ok = true;
    for (int iter = 0; iter < MAX; iter++) {
      if (!pworld->map.ValidToMove(w.x + d.DX() + wd.DX() * iter,
                                   w.y + d.DY() + wd.DY() * iter)) {
        ok = false;
      }
    }
    if (!ok) {
      continue;
    }
    Point base = pw;
    for (int iter = 0; iter < MAX; iter++) {
      int shift_results[4];
      if (iter > 0) {
        base = base + wd;
      }
      if (!pworld->map.ValidToMove(base.x, base.y)) {
        continue;
      }
      if (iter > 1 && pworld->map.Get(base.x, base.y).Wrapped()) {
        continue;
      }
      for (int shift = -1; shift < 3; shift++) {
        shift_results[shift + 1] =
            w.CellsToNewlyWrap(pworld->map, wd.DX() * iter + d.DX() * shift,
                               wd.DY() * iter + d.DY() * shift)
                .size();
      }
      if (shift_results[2] > shift_results[1]) {
        score++;  // Immediate gain
      } else if (shift_results[0] == shift_results[1] &&
                 shift_results[1] == shift_results[2] &&
                 shift_results[2] > shift_results[3]) {
        score++;
      }
    }
    if (score == MAX) {
      return Action(d.Get());
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
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
