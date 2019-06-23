#include "solvers/base_clones1.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/point.h"
#include "common/graph/graph/distance.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace solvers {
void BaseClones1::Init(const std::string& task, BaseClones1Settings _sett) {
  sett = _sett;
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
    if ((item == Item::CLONE) || (item == Item::CODEX)) {
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

void BaseClones1::CleanPOI() {
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

void BaseClones1::BuildDSUnsignedSet() {
  for (unsigned u : ds_rebuid.List()) {
    for (unsigned t : g.Edges(u)) {
      if ((t > u) && unwrapped.HasKey(t)) {
        ds.Union(u, t);
      }
    }
  }
}

void BaseClones1::BuildDS() {
  ds.Init(world.map.Size());
  ds_rebuid = unwrapped;
  BuildDSUnsignedSet();
  UpdateTarget();
}

void BaseClones1::RebuildDS() {
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

void BaseClones1::UpdateTarget() {
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

bool BaseClones1::AssignClosestWorker(unsigned r, ActionsList& al) {
  struct S {
    unsigned distance;
    unsigned index;
    unsigned from;
  };
  thread_local std::queue<S> q;
  thread_local std::unordered_map<unsigned, unsigned> m;
  for (int phase = 0; phase < 2; phase++) {
    for (; !q.empty();) q.pop();
    acw1.Clear();
    acw2.Clear();
    for (unsigned u : unwrapped.List()) {
      if (phase == 0 && !world.map.HasExtension(u)) {
        continue;
      }
      if (ds.Find(u) == r) {
        q.push({0, u, 0});
        acw1.Insert(u);
      }
    }
    if (phase == 0 && world.map.items_coords.count(Item::EXTENSION) > 0) {
      for (auto p : world.map.items_coords[Item::EXTENSION]) {
        unsigned u = world.map.Index(p.first, p.second);
        q.push({0, u, 0});
        acw1.Insert(u);
      }
    }
    m.clear();
    for (unsigned i = 0; i < world.WCount(); ++i) {
      auto& w = world.GetWorker(i);
      unsigned index = world.map.Index(w.x, w.y);
      // std::cout << w.x << " " << w.y << " " << world.map.xsize << " "
      //           << world.map.ysize << std::endl;
      acw2.Insert(index);
      m[index] = i;
    }
    unsigned best_distance = unsigned(-1);
    bool ok = false;
    Worker w = world.GetWorker(0);
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
          w = world.GetWorker(wi);
          Direction d = GetDirection(world.map, u, f);
          Worker w = world.GetWorker(wi);

          if (wi == sett.manip_index &&
              world.boosters.extensions.Available({world.time, wi})) {
            auto p = w.GetNextManipulatorPositionNaive(sett.strategy);
            al[wi].type = ActionType::ATTACH_MANIPULATOR;
            al[wi].x = p.first;
            al[wi].y = p.second;
            return true;
          }

          Direction wd = w.direction;
          Point pw(w.x, w.y);

          if (wi == 0 && sett.use_shifts) {
            for (int i = 0; i < 2; i++) {
              Direction d((1 + 2 * i + w.direction.direction) % 4);
              int score = 0;
              Point base = pw;
              Direction wd = w.direction;
              int MAX = 4;
              for (int iter = 0; iter < MAX; iter++) {
                if (iter > 0) {
                  base = base + wd;
                }
                if (!world.map.ValidToMove(base.x, base.y)) {
                  continue;
                }
                if (iter > 1 && world.map.Get(base.x, base.y).Wrapped()) {
                  continue;
                }
                int sz = w.CellsToNewlyWrap(world.map, wd.DX() * iter,
                                            wd.DY() * iter)
                             .size();
                Point pd = base + d;
                if (!world.map.ValidToMove(pd.x, pd.y)) {
                  continue;
                }
                int new_sz =
                    w.CellsToNewlyWrap(world.map, wd.DX() * iter + d.DX(),
                                       wd.DY() * iter + d.DY())
                        .size();
                if (new_sz > sz) {
                  score++;
                }
              }
              if (score == MAX) {
                // w.PrintNeighborhood(world.map, 4);
                al[wi].type = d.Get();
                return true;
              }
            }
          }

          if (d.direction != wd.direction && wi == sett.manip_index &&
              phase == 1) {
            bool need_turn = true;
            Point next = pw + d;
            for (int i = 0; i < 4; i++) {
              if (!world.map.Inside(next.x, next.y) ||
                  world.map.Get(next.x, next.y).WrappedOrBlocked()) {
                need_turn = false;
              }
              next = next + d;
            }
            if (need_turn) {
              if (d.direction == w.direction.direction - 1) {
                al[wi].type = ActionType::ROTATE_CLOCKWISE;
                return true;
              } else {
                al[wi].type = ActionType::ROTATE_COUNTERCLOCKWISE;
                return true;
              }
            }
          }
          al[wi].type = d.Get();
          return true;
        }
      }
      if (d < best_distance) {
        if (phase == 0 && d > sett.ext_dist) {
          continue;
        }
        for (unsigned v : g.Edges(u)) {
          if (!acw1.HasKey(v)) {
            acw1.Insert(v);
            q.push({d + 1, v, u});
          }
        }
      }
    }
  }
  return false;
}

Action BaseClones1::SendToNearestUnwrapped(unsigned windex) {
  auto& w = world.GetWorker(windex);
  if (windex == sett.manip_index &&
      world.boosters.extensions.Available({world.time, windex})) {
    auto p = w.GetNextManipulatorPositionNaive(sett.strategy);
    Action a(ActionType::ATTACH_MANIPULATOR, p.first, p.second);
    return a;
  }
  thread_local std::queue<std::pair<int, Direction>> q;
  for (; !q.empty();) q.pop();
  acw1.Clear();

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
    if (unwrapped.HasKey(index) || world.map.HasExtension(index)) {
      if (d.direction % 2 != w.direction.direction % 2 && windex == 0) {
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
          if (d.direction == w.direction.direction - 1) {
            return Action(ActionType::ROTATE_CLOCKWISE);
          } else {
            return Action(ActionType::ROTATE_COUNTERCLOCKWISE);
          }
        }
      }
      return Action(d.Get());
    }
    for (int inext : g.Edges(index)) {
      if (!acw1.HasKey(inext)) {
        q.push(std::make_pair(inext, d));
        acw1.Insert(inext);
      }
    }
  }
  return Action(ActionType::DO_NOTHING);
}

void BaseClones1::NextMove_Clone(ActionsList& al) {
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
        if (al[j].type != ActionType::DO_NOTHING) continue;
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
      assert((best_distance == 0) ||
             (al[windex].type != ActionType::DO_NOTHING));
    }
  }
}

void BaseClones1::NextMove_Wrap(ActionsList& al) {
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
    if (al[i].type == ActionType::DO_NOTHING) {
      al[i] = SendToNearestUnwrapped(i);
    }
  }
}

ActionsList BaseClones1::NextMove() {
  unsigned l = world.WCount();
  ActionsList al(l, Action(ActionType::DO_NOTHING));
  NextMove_Clone(al);
  NextMove_Wrap(al);
  world.ApplyC(al);
  Update();
  return al;
}

void BaseClones1::Update() {
  auto& q = world.map.wraps_history;
  for (; !q.empty(); q.pop()) {
    int index = q.front();
    if (unwrapped.HasKey(index)) {
      unwrapped.Remove(index);
      ds_rebuid_required.Insert(ds.Find(index));
    }
  }
}

bool BaseClones1::Wrapped() { return unwrapped.Empty(); }

ActionsClones BaseClones1::Solve(const std::string& task,
                                 BaseClones1Settings sett,
                                 const std::string& bonuses) {
  Init(task, sett);
  world.InitBonuses(bonuses);
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
  return actions;
}
}  // namespace solvers
