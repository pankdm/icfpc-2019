#include "solvers/base_greedy2.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/point.h"
#include "solvers/paths/path_to_target.h"
#include "common/always_assert.h"
#include "common/unsigned_set.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>

namespace solvers {
void BaseGreedy2::Init(const std::string& task) {
  world.Init(task);
  unsigned size = world.Size();
  target.Resize(size);
  SetTarget();
}

void BaseGreedy2::SetTarget(unsigned dist_weight) {
  if (dist_weight == 0) {
    unsigned min_ds_size = world.Size();
    for (unsigned u : world.UList()) {
      min_ds_size = std::min(min_ds_size, world.DSSize(u));
    }
    target.Clear();
    for (unsigned u : world.UList()) {
      if (world.DSSize(u) == min_ds_size) target.Insert(u);
    }
  } else {
    target.Clear();
    auto w = world.GetWorker();
    // score = dist + size
    int best_score = -1;
    std::map<int, int> rep_to_score;
    std::map<int, int> rep_to_size;
    for (unsigned u : world.UList()) {
      rep_to_size[world.DSFind(u)] = world.DSSize(u);
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
        int rep = world.DSFind(index);
        if (rep_to_score.count(rep) == 0) {
          int score = dist * dist_weight + world.DSSize(rep);
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
    for (unsigned u : world.UList()) {
      if (rep_to_score[world.DSFind(u)] == best_score) {
        target.Insert(u);
      }
    }
  }
}

Action BaseGreedy2::NextMove() {
  if (world.UpdateDSRequired()) {
    world.UpdateDS();
    SetTarget();
  }
  Action a = PathToTarget(world.GetWorker(), world, target);
  world.Apply(a);
  return a;
}

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
