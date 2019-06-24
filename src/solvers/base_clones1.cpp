#include "solvers/base_clones1.h"

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
void BaseClones1::Init(const std::string& task, BaseClones1Settings _sett) {
  sett = _sett;
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

  auto isBadAlpha = [&](double alpha) {
    int xmin = (0.5 - alpha) * world.map.xsize;
    int xmax = (0.5 + alpha) * world.map.xsize;
    int ymin = (0.5 - alpha) * world.map.ysize;
    int ymax = (0.5 + alpha) * world.map.ysize;
    for (int x = xmin; x <= xmax; ++x) {
      for (int y = ymin; y <= ymax; ++y) {
        if (world.map.Get(x, y).GetItem() == Item::NONE) {
          return false;
        }
      }
    }
    return true;
  };

  if (sett.use_teleports) {
    while (isBadAlpha(beaconAlpha)) {
      beaconAlpha *= 1.2;
    }
  }
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

void BaseClones1::UpdateTarget() {
  thread_local std::unordered_set<unsigned> s;
  s.clear();
  target.clear();
  for (unsigned u : ulist()) {
    unsigned p = world.DSFind(u);
    if (s.find(p) == s.end()) {
      s.insert(p);
      target.emplace_back(std::make_pair(world.DSSize(p), p));
    }
  }
  sort(target.begin(), target.end());
}

bool BaseClones1::AssignClosestWorker(unsigned r, ActionsList& al) {
  struct S {
    unsigned distance;
    unsigned index;
    unsigned from;
    unsigned source;
  };
  thread_local std::queue<S> q;
  thread_local std::unordered_map<unsigned, unsigned> m;
  for (int phase = 0; phase < 2; phase++) {
    for (; !q.empty();) q.pop();
    acw1.Clear();
    acw2.Clear();
    for (unsigned u : ulist()) {
      if (phase == 0 && !world.map.HasExtension(u)) {
        continue;
      }
      if (world.DSFind(u) == r) {
        q.push({0, u, 0, u});
        acw1.Insert(u);
      }
    }
    if (phase == 0 && world.map.items_coords.count(Item::EXTENSION) > 0) {
      for (auto p : world.map.items_coords[Item::EXTENSION]) {
        unsigned u = world.map.Index(p.first, p.second);
        q.push({0, u, 0, u});
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
      unsigned source = q.front().source;
      if (d > best_distance) break;
      if (acw2.HasKey(u)) {
        best_distance = d;
        unsigned wi = m[u];
        ALWAYS_ASSERT(wi < al.size());
        if (Sleep(al[wi])) {
          Action temp;
          if (NextMove_FastWheels(wi, temp)) {
            al[wi] = temp;
            return true;
          }

          if (NextMove_SetBeacon(wi, temp)) {
            al[wi] = temp;
            return true;
          }

          if (NextMove_Shift(wi, source, d, temp)) {
            al[wi] = temp;
            return true;
          }

          w = world.GetWorker(wi);
          Direction d = GetDirection(world.map, u, f);
          Worker w = world.GetWorker(wi);

          if (wi == next_mop_picker &&
              world.boosters.extensions.Available({world.time, wi})) {
            auto p = w.GetNextManipulatorPositionNaive(sett.strategy);
            if (sett.manip_index == -1) {
              next_mop_picker = (next_mop_picker + 1) % world.WCount();
            }
            world.boosters.extensions.LockUntilPicked();
            al[wi].type = ActionType::ATTACH_MANIPULATOR;
            al[wi].x = p.first;
            al[wi].y = p.second;
            return true;
          }

          Direction wd = w.direction;
          Point pw(w.x, w.y);

          if ((sett.is_manip(wi) || sett.all_rotate_and_shift) &&
              sett.use_shifts) {
            for (int i = 0; i < 2; i++) {
              Direction d((1 + 2 * i + w.direction.direction) % 4);
              int score = 0;
              Direction wd = w.direction;
              int MAX = 4;
              bool ok = true;
              for (int iter = 0; iter < MAX; iter++) {
                if (!world.map.ValidToMove(w.x + d.DX() + wd.DX() * iter,
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
                if (!world.map.ValidToMove(base.x, base.y)) {
                  continue;
                }
                if (iter > 1 && world.map.Get(base.x, base.y).Wrapped()) {
                  continue;
                }
                for (int shift = -1; shift < 3; shift++) {
                  shift_results[shift + 1] =
                      w.CellsToNewlyWrap(world.map,
                                         wd.DX() * iter + d.DX() * shift,
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
                al[wi].type = d.Get();
                return true;
              }
            }
          }

          if (d.direction != wd.direction &&
              (sett.is_manip(wi) || sett.all_rotate_and_shift) && phase == 1) {
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
        for (unsigned v : world.GEdges(u)) {
          if (!acw1.HasKey(v)) {
            acw1.Insert(v);
            q.push({d + 1, v, u, source});
          }
        }
      }
    }
  }
  return false;
}

bool BaseClones1::NextMove_SetBeacon(unsigned windex, Action& result) {
  if (sett.use_teleports) {
    const auto& w = world.GetWorker(windex);
    if (sett.is_manip(windex) &&
        world.boosters.teleporters.Available({world.time, windex}) &&
        !reset_beacon) {
      auto atCenter = [&](int x, int y, int xsize, int ysize) {
        auto atCenter1 = [&](int x, int xsize) {
          return x > ((0.5 - beaconAlpha) * xsize) &&
                 x < (0.5 + beaconAlpha * xsize);
        };

        return atCenter1(x, xsize) && atCenter1(y, ysize);
      };

      if (atCenter(w.x, w.y, world.map.xsize, world.map.ysize) &&
          world.map.Get(w.x, w.y).CheckItem() == Item::NONE) {
        Action a(ActionType::SET_BEACON, w.x, w.y);
        beacon = {w.x, w.y};
        reset_beacon = true;

        beaconDist.resize(world.map.Size(), -1);
        beaconDist[world.map.Index(w.x, w.y)] = 0;
        std::queue<int> q;
        q.push(world.map.Index(w.x, w.y));

        while (!q.empty()) {
          int now = q.front();
          Point pnow(world.map.X(now), world.map.Y(now));
          q.pop();

          for (unsigned _d = 0; _d < 4; ++_d) {
            Direction d(_d);
            Point pd = pnow + d;
            if (world.map.ValidToMove(pd.x, pd.y)) {
              int pdi = world.map.Index(pd.x, pd.y);
              if (beaconDist[pdi] == -1) {
                beaconDist[pdi] = beaconDist[now] + 1;
                q.push(pdi);
              }
            }
          }
        }

        result = a;
        return true;
      }
    }
  }

  return false;
}

bool BaseClones1::NextMove_FastWheels(unsigned windex, Action& result) {
  if (sett.use_fast_wheels && !fast_wheels) {
    const auto& w = world.GetWorker(windex);
    if (sett.is_manip(windex) &&
        world.boosters.fast_wheels.Available({world.time, windex})) {
      Action a(ActionType::ATTACH_FAST_WHEELS, w.x, w.y);
      result = a;
      fast_wheels = true;
      return true;
    }
  }

  return false;
}

bool BaseClones1::NextMove_Shift(unsigned windex, unsigned dest_index,
                                 unsigned now_distance, Action& action) {
  if (sett.use_teleports && reset_beacon) {
    // std::cerr << beaconDist[dest_index] << " " << now_distance << std::endl;
    auto& w = world.GetWorker(windex);
    if (beaconDist[dest_index] + 2 < now_distance) {
      /*
      std::cerr << "Use beacon at (" << beacon.x << ", " << beacon.y
                << ") to get from (" << w.x << ", " << w.y << ") "
                << " to (" << world.map.X(dest_index) << ", "
                << world.map.Y(dest_index) << ")"
                << " now_dist: " << now_distance
                << " beacon_dist: " << beaconDist[dest_index] << std::endl;
      */
      action = Action(ActionType::SHIFT, beacon.x, beacon.y);
      return true;
    }
  }
  return false;
}

Action BaseClones1::SendToNearestUnwrapped(unsigned windex) {
  auto& w = world.GetWorker(windex);
  if (windex == next_mop_picker &&
      world.boosters.extensions.Available({world.time, windex})) {
    auto p = w.GetNextManipulatorPositionNaive(sett.strategy);
    if (sett.manip_index == -1) {
      next_mop_picker = (next_mop_picker + 1) % world.WCount();
    }
    world.boosters.extensions.LockUntilPicked();
    Action a(ActionType::ATTACH_MANIPULATOR, p.first, p.second);
    return a;
  }

  Action temp;
  if (NextMove_SetBeacon(windex, temp)) {
    return temp;
  }

  struct QItem {
    int pointIndex;
    Direction d;
    int distance;
  };

  thread_local std::queue<QItem> q;
  for (; !q.empty();) {
    q.pop();
  }
  acw1.Clear();

  Point pw(w.x, w.y);
  for (unsigned _d = 0; _d < 4; ++_d) {
    Direction d(_d);
    Point pd = pw + d;
    if (world.map.ValidToMove(pd.x, pd.y)) {
      int index = world.map.Index(pd.x, pd.y);
      q.push(QItem{index, d, 1});
      acw1.Insert(index);
    }
  }
  for (; !q.empty(); q.pop()) {
    QItem now = q.front();
    int index = now.pointIndex;
    ;
    Direction d = now.d;
    if (world.Unwrapped(index) || world.map.HasExtension(index)) {
      Action temp;
      if (NextMove_Shift(windex, index, now.distance, temp)) {
        return temp;
      }

      if (d.direction % 2 != w.direction.direction % 2 &&
          sett.is_manip(windex)) {
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
    for (int inext : world.GEdges(index)) {
      if (!acw1.HasKey(inext)) {
        q.push(QItem{inext, d, now.distance + 1});
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
      ALWAYS_ASSERT((best_distance == 0) || !Sleep(al[windex].type));
    }
  }
}

void BaseClones1::NextMove_Wrap(ActionsList& al) {
  unsigned l = al.size();
  unsigned waiting_workers = 0;
  for (auto& action : al) {
    if (Sleep(action)) {
      waiting_workers += 1;
    }
  }
  if (waiting_workers == 0) return;
  if (world.UpdateDSRequired()) {
    world.UpdateDS();
    UpdateTarget();
  }
  for (auto& tp : target) {
    if (waiting_workers == 0) {
      break;
    }
    if (AssignClosestWorker(tp.second, al)) {
      waiting_workers -= 1;
    }
  }
  // Do something with remaining workers
  for (unsigned i = 0; i < l; ++i) {
    if (Sleep(al[i])) {
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
  return al;
}

std::vector<unsigned> BaseClones1::ulist() const {
    auto result = world.UList();
    if (sett.sorted_points) {
        std::sort(result.begin(), result.end());
    }
    return result;
}

ActionsClones BaseClones1::Solve(const std::string& task,
                                 BaseClones1Settings sett,
                                 const std::string& bonuses) {
  Init(task, sett);
  if (sett.manip_index == -1) {
    next_mop_picker = 0;
  } else {
    next_mop_picker = sett.manip_index;
  }
  world.InitBonuses(bonuses);
  ActionsClones actions;
  for (; !world.Solved();) {
    auto al = NextMove();
    if (Sleep(al)) break;
    if (actions.size() < al.size()) {
      actions.resize(al.size());
    }
    for (unsigned i = 0; i < al.size(); ++i) actions[i].emplace_back(al[i]);
  }
  if (actions.size() < world.WCount()) {
    actions.resize(world.WCount());
  }
  return actions;
}

}  // namespace solvers
