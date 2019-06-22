#include "base/world.h"

#include "base/item.h"
#include "base/map.h"
#include "base/point.h"
#include "base/worker.h"
#include "utils/split.h"
#include "common/always_assert.h"
#include <cassert>
#include <vector>

void World::Init(const std::string& desc) {
  time = 0;
  auto vs = Split(desc, '#');
  ALWAYS_ASSERT(vs.size() == 4);
  map.Init(vs[0]);
  for (auto& block_desc : Split(vs[2], ';')) map.AddBlock(block_desc);
  for (auto& boost_desc : Split(vs[3], ';')) {
    assert(boost_desc.size() >= 1);
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
    assert(item != Item::UNKNOWN);
    map(pboost.x, pboost.y).AddItem(item);
  }
  Point pworker(vs[1]);
  workers.resize(1);
  GetWorker().Init(boosters, map, pworker.x, pworker.y);
}

Worker& World::GetWorker(unsigned index) {
  assert(index < workers.size());
  return workers[index];
}

void World::Apply(const Action& action) {
  GetWorker().Apply(++time, map, action);
}

void World::Apply(const ActionsList& actions) {
  for (const Action& action : actions) Apply(action);
}

bool World::Solved() const { return map.Wrapped(); }

void World::ApplyC(unsigned index, const Action& action) {
  if (action.type == ActionType::CLONE) {
    auto& worker = GetWorker(index);
    assert(boosters.unused_clones > 0);
    assert(map.Get(worker.x, worker.y).CheckItem() == Item::CODEX);
    boosters.unused_clones -= 1;
    workers.emplace_back(worker);
  } else {
    GetWorker(index).Apply(time, map, action);
  }
}

void World::ApplyC(const ActionsClones& actions) {
  unsigned max_workers = actions.size();
  std::vector<unsigned> vindex(max_workers, 0);
  for (bool last = false; !last;) {
    ++time;
    last = true;
    unsigned l = workers.size();
    for (unsigned i = 0; i < l; ++i) {
      if (vindex[i] < actions[i].size()) {
        ApplyC(i, actions[i][vindex[i]++]);
        last = false;
      }
    }
  }
}
