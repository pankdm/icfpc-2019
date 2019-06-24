#include "solvers/base_clones.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/point.h"
#include "base/sleep.h"
#include "common/always_assert.h"
#include "common/graph/graph/distance.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace solvers {
void BaseClones::Init(const std::string& task) {
  world.Init(task);
  unsigned size = world.Size();
  auto& map = world.map;
  for (int index = 0; index < map.Size(); ++index) {
    Item item = map[index].CheckItem();
    if ((item == Item::CLONE) || (item == Item::CODEX)) {
      poi.emplace_back(POI{item, index, DistanceFromSource(world.G(), index)});
    }
  }
  poi_assigned.Resize(poi.size());
  acw1.Resize(size);
  acw2.Resize(size);
  UpdateTarget();
}

void BaseClones::CleanPOI() {
  for (unsigned i = 0; i < poi.size();) {
    Item item = world.map[poi[i].index].CheckItem();
    if ((item == Item::CLONE) || (item == Item::CODEX)) {
      ++i;
    } else {
      if (i + 1 < poi.size()) std::swap(poi[i], poi.back());
      poi.pop_back();
    }
  }
}

void BaseClones::UpdateTarget() {
  thread_local std::unordered_set<unsigned> s;
  s.clear();
  target.clear();
  for (unsigned u : world.UList()) {
    unsigned p = world.DSFind(u);
    if (s.find(p) == s.end()) {
      s.insert(p);
      target.emplace_back(std::make_pair(world.DSSize(p), p));
    }
  }
  sort(target.begin(), target.end());
}

bool BaseClones::AssignClosestWorker(unsigned r, ActionsList& al) {
  struct S {
    unsigned distance;
    unsigned index;
    unsigned from;
  };
  thread_local std::queue<S> q;
  thread_local std::unordered_map<unsigned, unsigned> m;
  for (; !q.empty();) q.pop();
  acw1.Clear();
  acw2.Clear();
  for (unsigned u : world.UList()) {
    if (world.DSFind(u) == r) {
      q.push({0, u, 0});
      acw1.Insert(u);
    }
  }
  m.clear();
  for (unsigned i = 0; i < world.WCount(); ++i) {
    auto& w = world.GetWorker(i);
    unsigned index = world.map.Index(w.x, w.y);
    acw2.Insert(index);
    m[index] = i;
  }
  unsigned best_distance = unsigned(-1);
  for (; !q.empty(); q.pop()) {
    unsigned d = q.front().distance;
    unsigned u = q.front().index;
    unsigned f = q.front().from;
    if (d > best_distance) break;
    if (acw2.HasKey(u)) {
      best_distance = d;
      unsigned wi = m[u];
      ALWAYS_ASSERT(wi < al.size());
      if (Sleep(al[wi])) {
        al[wi].type = GetDirection(world.map, u, f).Get();
        return true;
      }
    }
    if (d < best_distance) {
      for (unsigned v : world.GEdges(u)) {
        if (!acw1.HasKey(v)) {
          acw1.Insert(v);
          q.push({d + 1, v, u});
        }
      }
    }
  }
  return false;
}

ActionType BaseClones::SendToNearestUnwrapped(unsigned windex) {
  thread_local std::queue<std::pair<int, Direction>> q;
  for (; !q.empty();) q.pop();
  acw1.Clear();
  auto& w = world.GetWorker(windex);
  Point pw(w.x, w.y);
  for (unsigned _d = 0; _d < 4; ++_d) {
    Direction d(_d);
    Point pd = pw + d;
    if (world.map.ValidToMove(pd.x, pd.y)) {
      int index = world.map.Index(pd.x, pd.y);
      q.push(std::make_pair(index, d));
      acw1.Insert(index);
    }
  }
  for (; !q.empty(); q.pop()) {
    int index = q.front().first;
    Direction d = q.front().second;
    if (world.Unwrapped(index)) return d.Get();
    for (int inext : world.GEdges(index)) {
      if (!acw1.HasKey(inext)) {
        q.push(std::make_pair(inext, d));
        acw1.Insert(inext);
      }
    }
  }
  return ActionType::DO_NOTHING;
}

void BaseClones::NextMove_Clone(ActionsList& al) {
  unsigned l = al.size();
  CleanPOI();
  unsigned new_workers = 0;
  if (world.boosters.clones.Available({world.time, 0})) {
    for (unsigned i = 0; i < l; ++i) {
      auto& w = world.GetWorker(i);
      if (world.map(w.x, w.y).CheckItem() == Item::CODEX) {
        al[i].type = ActionType::CLONE;
        new_workers += 1;
        break;
      }
    }
  }
  bool unused_boosters = (world.boosters.clones.Size() > new_workers);
  poi_assigned.Clear();
  for (bool assigned = true; assigned;) {
    assigned = false;
    unsigned best_distance = unsigned(-1);
    unsigned pindex = 0, windex = 0;
    for (unsigned i = 0; i < poi.size(); ++i) {
      if (poi_assigned.HasKey(i) ||
          (!unused_boosters && (poi[i].item == Item::CODEX)))
        continue;
      for (unsigned j = 0; j < l; ++j) {
        if (!Sleep(al[j])) continue;
        auto& w = world.GetWorker(j);
        int mindex = world.map.Index(w.x, w.y);
        unsigned distance = poi[i].vd[mindex];
        if (distance < best_distance) {
          best_distance = distance;
          pindex = i;
          windex = j;
        }
      }
    }
    if (best_distance != unsigned(-1)) {
      assigned = true;
      poi_assigned.Insert(pindex);
      if (best_distance > 0) {
        auto& w = world.GetWorker(windex);
        Point pw(w.x, w.y);
        for (unsigned _d = 0; _d < 4; ++_d) {
          Direction d(_d);
          Point pd = pw + d;
          if (world.map.ValidToMove(pd.x, pd.y)) {
            int pdi = world.map.Index(pd.x, pd.y);
            if (best_distance == poi[pindex].vd[pdi] + 1) {
              al[windex].type = d.Get();
              break;
            }
          }
        }
      }
      ALWAYS_ASSERT((best_distance == 0) || !Sleep(al[windex]));
    }
  }
}

void BaseClones::NextMove_Wrap(ActionsList& al) {
  unsigned l = al.size();
  unsigned waiting_workers = 0;
  for (auto& action : al) {
    if (Sleep(action)) waiting_workers += 1;
  }
  if (waiting_workers == 0) return;
  if (world.UpdateDSRequired()) {
    world.UpdateDS();
    UpdateTarget();
  }
  for (auto& tp : target) {
    if (waiting_workers == 0) break;
    if (AssignClosestWorker(tp.second, al)) waiting_workers -= 1;
  }
  // Do something with remaining workers
  for (unsigned i = 0; i < l; ++i) {
    if (Sleep(al[i])) al[i].type = SendToNearestUnwrapped(i);
  }
}

ActionsList BaseClones::NextMove() {
  unsigned l = world.WCount();
  ActionsList al(l, Action(ActionType::DO_NOTHING));
  NextMove_Clone(al);
  NextMove_Wrap(al);
  world.ApplyC(al);
  return al;
}

ActionsClones BaseClones::Solve(const std::string& task,
                                const std::string& bonuses) {
  world.InitBonuses(bonuses);
  Init(task);
  ActionsClones actions;
  for (; !world.Solved();) {
    auto al = NextMove();
    if (Sleep(al)) break;
    if (actions.size() < al.size()) {
      actions.resize(al.size());
    }
    for (unsigned i = 0; i < al.size(); ++i) actions[i].emplace_back(al[i]);
  }
  if (actions.size() < world.WCount()) actions.resize(world.WCount());
  return actions;
}
}  // namespace solvers
