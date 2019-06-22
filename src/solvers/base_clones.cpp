#include "solvers/base_clones.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/point.h"
#include "common/graph/graph/distance.h"
#include <algorithm>

#include <iostream>

namespace solvers {
void BaseClones::Init(const std::string& task) {
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

ActionsList BaseClones::NextMove() {
  //   std::cerr << "BaseClones::NextMove()" << std::endl;
  //   std::cerr << "\tT = " << world.time << "\tL = " << world.WCount()
  //             << std::endl;
  CleanPOI();
  unsigned l = world.WCount();
  ActionsList al(l, Action(ActionType::DO_NOTHING));
  // Check if we can create new worker
  unsigned new_workers = 0;
  if (world.boosters.unused_clones > 0) {
    for (unsigned i = 0; i < l; ++i) {
      auto& w = world.GetWorker(i);
      if (world.map(w.x, w.y).CheckItem() == Item::CODEX) {
        al[i].type = ActionType::CLONE;
        new_workers += 1;
        // std::cerr << "\tCloning worker " << i << " @ (" << w.x << ", " << w.y
        //           << ")" << std::endl;
        break;
      }
    }
  }
  bool unused_boosters = (world.boosters.unused_clones > new_workers);
  std::vector<unsigned> vassgined(poi.size(), 0);  // Use UnsignedSet later
  //   std::cerr << "\tPOI size = " << poi.size() << std::endl;
  for (bool assigned = true; assigned;) {
    assigned = false;
    unsigned best_distance = unsigned(-1);
    unsigned pindex = 0, windex = 0;
    for (unsigned i = 0; i < poi.size(); ++i) {
      if (vassgined[i] || (!unused_boosters && (poi[i].item == Item::CODEX)))
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
      vassgined[pindex] = 1;
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
      //   std::cerr << "\tWorker " << windex << " @ (" << w.x << ", " << w.y
      //             << ") assigned to poi " << pindex << " @ " <<
      //             poi[pindex].index
      //             << std::endl;
    }
  }
  bool stop = true;
  for (Action& a : al) {
    if (a.type != ActionType::DO_NOTHING) {
      stop = false;
      break;
    }
  }
  if (!stop) {
    world.ApplyC(al);
    return al;
  }
  return {};
}

ActionsClones BaseClones::Solve(const std::string& task) {
  Init(task);
  ActionsClones actions;
  for (;;) {
    auto al = NextMove();
    if (al.size() == 0) break;
    if (actions.size() < al.size()) {
      actions.resize(al.size());
    }
    for (unsigned i = 0; i < al.size(); ++i) {
      actions[i].emplace_back(al[i]);
    }
  }
  if (actions.size() < world.WCount()) actions.resize(world.WCount());
  //   std::cerr << "Solver finished" << std::endl;
  return actions;
}
}  // namespace solvers
