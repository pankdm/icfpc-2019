#include "solvers/clones_greedy.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/count.h"
#include "base/direction.h"
#include "base/item.h"
#include "base/point.h"
#include "base/sleep.h"
#include "solvers/paths/path_to_target.h"
#include "common/always_assert.h"
#include "common/graph/graph/distance.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace solvers {
void ClonesGreedy::Init(const std::string& task) {
  world.Init(task);
  unsigned size = world.Size();
  auto& map = world.map;
  codex_set.Resize(size);
  for (int index = 0; index < map.Size(); ++index) {
    Item item = map[index].CheckItem();
    if ((item == Item::CLONE) || (item == Item::EXTENSION)) {
      poi.emplace_back(POI{item, index, DistanceFromSource(world.G(), index)});
    }
    if (item == Item::CODEX) codex_set.Insert(index);
  }
  poi_assigned.Resize(poi.size());
  acw1.Resize(size);
  acw2.Resize(size);
  UpdateTarget();
}

void ClonesGreedy::CleanPOI() {
  for (unsigned i = 0; i < poi.size();) {
    Item item = world.map[poi[i].index].CheckItem();
    if (item != Item::NONE) {
      ++i;
    } else {
      if (i + 1 < poi.size()) std::swap(poi[i], poi.back());
      poi.pop_back();
    }
  }
}

void ClonesGreedy::UpdateTarget() {
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

bool ClonesGreedy::AssignClosestWorker(unsigned r, ActionsList& al) {
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

void ClonesGreedy::NextMove_Init(ActionsList& al) {
  unsigned l = al.size();
  auto& w = world.GetWorker(0);
  unsigned windex = world.map.Index(w.x, w.y);
  // If we can apply extension, apply.
  if (world.boosters.extensions.Available({world.time, 0})) {
    ALWAYS_ASSERT(l == 1);
    al[0].type = ActionType::ATTACH_MANIPULATOR;
    auto p = w.GetNextManipulatorPositionNaive(strategy);
    al[0].x = p.first;
    al[0].y = p.second;
    return;
  }
  CleanPOI();
  // If we didn't take something take nearest.
  if (poi.size() > 0) {
    ALWAYS_ASSERT(l == 1);
    unsigned best_distance = unsigned(-1);
    unsigned closest_poi = 0;
    for (unsigned i = 0; i < poi.size(); ++i) {
      if (poi[i].vd[windex] < best_distance) {
        best_distance = poi[i].vd[windex];
        closest_poi = i;
      }
    }
    ALWAYS_ASSERT((best_distance > 0) && (best_distance < unsigned(-1)));
    Point pw(w.x, w.y);
    for (unsigned _d = 0; _d < 4; ++_d) {
      Direction d(_d);
      Point pd = pw + d;
      if (world.map.ValidToMove(pd.x, pd.y)) {
        int pdi = world.map.Index(pd.x, pd.y);
        if (best_distance == poi[closest_poi].vd[pdi] + 1) {
          al[0].type = d.Get();
          break;
        }
      }
    }
    ALWAYS_ASSERT(!Sleep(al[0]));
    return;
  }
  // Go to nearest CodeX point.
  if ((world.boosters.clones.Size() > 0) &&
      (world.map[windex].CheckItem() != Item::CODEX)) {
    ALWAYS_ASSERT(l == 1);
    al[0] = PathToTarget(w, world, codex_set);
    return;
  }
  // Make clones
  unsigned clones = std::min(world.boosters.clones.Size(), l);
  for (unsigned i = 0; i < clones; ++i) {
    al[i].type = ActionType::CLONE;
  }
}

void ClonesGreedy::NextMove_Wrap(ActionsList& al) {
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
    if (Sleep(al[i]))
      al[i] = PathToTarget(world.GetWorker(i), world, world.UnwrappedSet());
  }
}

ActionsList ClonesGreedy::NextMove() {
  unsigned l = world.WCount();
  ActionsList al(l, Action(ActionType::DO_NOTHING));
  NextMove_Init(al);
  NextMove_Wrap(al);
  world.ApplyC(al);
  return al;
}

ActionsClones ClonesGreedy::Solve(const std::string& task, unsigned _strategy,
                                  const std::string& bonuses) {
  if ((Count(task, Item::CLONE) == 0) || (Count(task, Item::CODEX) == 0))
    return {};
  strategy = _strategy;
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
  //   std::cerr << "Solver finished" << std::endl;
  return actions;
}
}  // namespace solvers
