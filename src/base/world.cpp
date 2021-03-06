#include "base/world.h"

#include "base/item.h"
#include "base/map.h"
#include "base/point.h"
#include "base/worker.h"
#include "utils/split.h"
#include "common/always_assert.h"
#include <vector>

void World::Init(const std::string& desc) {
  time = 0;
  auto vs = Split(desc, '#');
  ALWAYS_ASSERT(vs.size() == 4);
  map.Init(vs[0]);
  for (auto& block_desc : Split(vs[2], ';')) map.AddBlock(block_desc);
  for (auto& boost_desc : Split(vs[3], ';')) {
    ALWAYS_ASSERT(boost_desc.size() >= 1);
    Point pboost(boost_desc.substr(1));
    Item item = Item::UNKNOWN;
    switch (boost_desc[0]) {
      case 'B':
        item = Item::EXTENSION;
        break;
      case 'F':
        item = Item::FAST_WHEELS;
        break;
      case 'L':
        item = Item::DRILL;
        break;
      case 'X':
        item = Item::CODEX;
        break;
      case 'R':
        item = Item::TELEPORT;
        break;
      case 'C':
        item = Item::CLONE;
        break;
    }
    ALWAYS_ASSERT(item != Item::UNKNOWN);
    map(pboost.x, pboost.y).AddItem(item);
    if (map.items_coords.count(item) == 0) {
      map.items_coords[item] = std::set<std::pair<int, int>>();
    }
    map.items_coords[item].insert(std::make_pair(pboost.x, pboost.y));
  }
  Point pworker(vs[1]);
  workers.resize(1);
  GetWorker().Init(boosters, map, pworker.x, pworker.y, 0);
}

void World::InitBonuses(const std::string& bonuses) {
  for (auto& b : bonuses) {
    switch (b) {
      case 'B':
        boosters.extensions.Add(BoosterTime());
        break;
      case 'F':
        boosters.fast_wheels.Add(BoosterTime());
        break;
      case 'L':
        boosters.drills.Add(BoosterTime());
        break;
      case 'R':
        boosters.teleporters.Add(BoosterTime());
        break;
      case 'C':
        boosters.clones.Add(BoosterTime());
        break;
      default:
        ALWAYS_ASSERT(false);
    }
  }
}

Worker& World::GetWorker(unsigned index) {
  ALWAYS_ASSERT(index < workers.size());
  return workers[index];
}

void World::Apply(const Action& action) {
  GetWorker().Apply(++time, map, action);
}

void World::Apply(const ActionsList& actions) {
  for (const Action& action : actions) Apply(action);
}

bool World::Solved() const {
  for (const auto& w : workers) {
    if (!w.valid) {
      return false;
    }
  }
  return map.Wrapped();
}

unsigned World::WCount() const { return workers.size(); }

void World::ApplyC(unsigned index, const Action& action) {
  if (action.type == ActionType::CLONE) {
    auto& worker = GetWorker(index);
    ALWAYS_ASSERT(boosters.clones.Available(worker.Time(time)));
    ALWAYS_ASSERT(map.Get(worker.x, worker.y).CheckItem() == Item::CODEX);
    boosters.clones.Use();
    Worker wnew;
    wnew.Init(boosters, map, worker.x, worker.y, workers.size());
    workers.emplace_back(wnew);
  } else {
    GetWorker(index).Apply(time, map, action);
  }
}

void World::ApplyC(const ActionsList& actions) {
  ALWAYS_ASSERT(actions.size() == WCount());
  ++time;
  for (unsigned i = 0; i < actions.size(); ++i) {
    ApplyC(i, actions[i]);
  }
}

void World::ApplyC(const ActionsClones& actions) {
  unsigned max_workers = actions.size();
  std::vector<unsigned> vindex(max_workers, 0);
  for (bool last = false; !last;) {
    ++time;
    last = true;
    unsigned l = WCount();
    for (unsigned i = 0; i < l; ++i) {
      if (vindex[i] < actions[i].size()) {
        ApplyC(i, actions[i][vindex[i]++]);
        last = false;
      }
    }
  }
}
