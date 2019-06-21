#include "base/world.h"

#include "base/item.h"
#include "base/map.h"
#include "base/point.h"
#include "base/worker.h"
#include "utils/split.h"
#include <cassert>

void World::Init(const std::string& desc) {
  time = 0;
  auto vs = Split(desc, '#');
  assert(vs.size() == 4);
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
    }
    assert(item != Item::UNKNOWN);
    map(pboost.x, pboost.y).AddItem(item);
  }
  Point pworker(vs[1]);
  worker.Init(map, pworker.x, pworker.y);
}

void World::Apply(const Action& action) { worker.Apply(++time, map, action); }

void World::Apply(const ActionsList& actions) {
  for (const Action& action : actions) Apply(action);
}

bool World::Solved() const { return map.Wrapped(); }
