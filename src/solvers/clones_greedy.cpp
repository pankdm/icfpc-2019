#include "solvers/clones_greedy.h"

#include "base/action.h"
#include "base/action_encode.h"
#include "base/action_type.h"
#include "base/count.h"
#include "base/direction.h"
#include "base/item.h"
#include "base/point.h"
#include "common/graph/graph/distance.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace solvers {
void ClonesGreedy::Init(const std::string& task) {
  world.Init(task);
  unsigned size = unsigned(world.map.Size());
  auto& map = world.map;
  map.save_wraps = true;
  g.Resize(size);
  unwrapped.Clear();
  unwrapped.Resize(size);
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
  for (int index = 0; index < map.Size(); ++index) {
    Item item = map[index].CheckItem();
    if ((item == Item::CLONE) || (item == Item::EXTENSION)) {
      poi.emplace_back(POI{item, index, DistanceFromSource(g, index)});
    }
  }
  poi_assigned.Resize(poi.size());
  ds_rebuid_required.Resize(size);
  ds_rebuid.Resize(size);
  acw1.Resize(size);
  acw2.Resize(size);
  BuildDS();
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

void ClonesGreedy::BuildDSUnsignedSet() {
  for (unsigned u : ds_rebuid.List()) {
    for (unsigned t : g.Edges(u)) {
      if ((t > u) && unwrapped.HasKey(t)) {
        ds.Union(u, t);
      }
    }
  }
}

void ClonesGreedy::BuildDS() {
  ds.Init(world.map.Size());
  ds_rebuid = unwrapped;
  BuildDSUnsignedSet();
  UpdateTarget();
}

void ClonesGreedy::RebuildDS() {
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

void ClonesGreedy::UpdateTarget() {
  thread_local std::unordered_set<unsigned> s;
  s.clear();
  target.clear();
  for (unsigned u : unwrapped.List()) {
    unsigned p = ds.Find(u);
    if (s.find(p) == s.end()) {
      s.insert(p);
      target.emplace_back(std::make_pair(ds.GetSize(p), p));
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
  for (unsigned u : unwrapped.List()) {
    if (ds.Find(u) == r) {
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
      assert(wi < al.size());
      if (al[wi].type == ActionType::DO_NOTHING) {
        al[wi].type = GetDirection(world.map, u, f).Get();
        return true;
      }
    }
    if (d < best_distance) {
      for (unsigned v : g.Edges(u)) {
        if (!acw1.HasKey(v)) {
          acw1.Insert(v);
          q.push({d + 1, v, u});
        }
      }
    }
  }
  return false;
}

ActionType ClonesGreedy::SendToNearestCodeX(unsigned windex) {
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
    if (world.map[index].CheckItem() == Item::CODEX) return d.Get();
    for (int inext : g.Edges(index)) {
      if (!acw1.HasKey(inext)) {
        q.push(std::make_pair(inext, d));
        acw1.Insert(inext);
      }
    }
  }
  return ActionType::DO_NOTHING;
}

ActionType ClonesGreedy::SendToNearestUnwrapped(unsigned windex) {
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
    if (unwrapped.HasKey(index)) return d.Get();
    for (int inext : g.Edges(index)) {
      if (!acw1.HasKey(inext)) {
        q.push(std::make_pair(inext, d));
        acw1.Insert(inext);
      }
    }
  }
  return ActionType::DO_NOTHING;
}

void ClonesGreedy::NextMove_Init(ActionsList& al) {
  unsigned l = al.size();
  auto& w = world.GetWorker(0);
  unsigned windex = world.map.Index(w.x, w.y);
  // If we can apply extension, apply.
  if (world.boosters.extensions.Available({world.time, 0})) {
    assert(l == 1);
    al[0].type = ActionType::ATTACH_MANIPULATOR;
    auto p = w.GetNextManipulatorPositionNaive(strategy);
    al[0].x = p.first;
    al[0].y = p.second;
    return;
  }
  CleanPOI();
  // If we didn't take something take nearest.
  if (poi.size() > 0) {
    assert(l == 1);
    unsigned best_distance = unsigned(-1);
    unsigned closest_poi = 0;
    for (unsigned i = 0; i < poi.size(); ++i) {
      if (poi[i].vd[windex] < best_distance) {
        best_distance = poi[i].vd[windex];
        closest_poi = i;
      }
    }
    assert((best_distance > 0) && (best_distance < unsigned(-1)));
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
    assert(al[0].type != ActionType::DO_NOTHING);
    return;
  }
  // Go to nearest CodeX point.
  if ((world.boosters.clones.Size() > 0) &&
      (world.map[windex].CheckItem() != Item::CODEX)) {
    assert(l == 1);
    al[0].type = SendToNearestCodeX(0);
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
    if (action.type == ActionType::DO_NOTHING) waiting_workers += 1;
  }
  if (waiting_workers == 0) return;
  if (!ds_rebuid_required.Empty()) {
    RebuildDS();
    ds_rebuid_required.Clear();
    UpdateTarget();
  }
  for (auto& tp : target) {
    if (waiting_workers == 0) break;
    if (AssignClosestWorker(tp.second, al)) waiting_workers -= 1;
  }
  // Do something with remaining workers
  for (unsigned i = 0; i < l; ++i) {
    if (al[i].type == ActionType::DO_NOTHING)
      al[i].type = SendToNearestUnwrapped(i);
  }
}

ActionsList ClonesGreedy::NextMove() {
  unsigned l = world.WCount();
  ActionsList al(l, Action(ActionType::DO_NOTHING));
  NextMove_Init(al);
  NextMove_Wrap(al);
  world.ApplyC(al);
  Update();
  return al;
}

void ClonesGreedy::Update() {
  auto& q = world.map.wraps_history;
  for (; !q.empty(); q.pop()) {
    int index = q.front();
    if (unwrapped.HasKey(index)) {
      unwrapped.Remove(index);
      ds_rebuid_required.Insert(ds.Find(index));
    }
  }
}

bool ClonesGreedy::Wrapped() { return unwrapped.Empty(); }

ActionsClones ClonesGreedy::Solve(const std::string& task, unsigned _strategy) {
  if ((Count(task, Item::CLONE) == 0) || (Count(task, Item::CODEX) == 0))
    return {};
  strategy = _strategy;
  Init(task);
  ActionsClones actions;
  for (; !Wrapped();) {
    auto al = NextMove();
    if (al.size() == 0) break;
    if (actions.size() < al.size()) {
      actions.resize(al.size());
    }
    bool do_nothing = true;
    for (unsigned i = 0; i < al.size(); ++i) {
      if (al[i].type != ActionType::DO_NOTHING) do_nothing = false;
      actions[i].emplace_back(al[i]);
    }
    if (do_nothing) break;
  }
  if (actions.size() < world.WCount()) actions.resize(world.WCount());
  //   std::cerr << "Solver finished" << std::endl;
  return actions;
}
}  // namespace solvers
