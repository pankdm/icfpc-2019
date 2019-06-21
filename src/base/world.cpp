#include "base/world.h"

#include "base/item.h"
#include "base/map.h"
#include "base/point.h"
#include "base/worker.h"
#include "utils/split.h"
#include <cassert>

#include <iostream>

void World::Init(const std::string& desc) {
  time = 0;
  auto vs = Split(desc, '#');
  //   std::cerr << "World: [" << desc << "]" << std::endl;
  //   std::cerr << "VS size = " << vs.size() << std::endl;
  //   for (auto& s : vs) std::cerr << "\t[" << s << "]" << std::endl;
  assert(vs.size() == 4);
  map.Init(vs[0]);
  //   std::cerr << "Map inititalized 1" << std::endl;
  // map.Print();
  for (auto& block_desc : Split(vs[2], ';')) map.AddBlock(block_desc);
  //   std::cerr << "Map inititalized 2" << std::endl;
  //   map.Print();
  //   std::cout << std::endl;
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
    // std::cerr << "\tAddItem(" << pboost.x << ", " << pboost.y << ", "
    //           << unsigned(item) << ")" << std::endl;
    map(pboost.x, pboost.y).AddItem(item);
  }
  Point pworker(vs[1]);
  worker.Init(map, pworker.x, pworker.y);
  //   std::cerr << "World initialized" << std::endl;
}

void World::Apply(const Action& action) { worker.Apply(++time, map, action); }

void World::Apply(const ActionsList& actions) {
  for (const Action& action : actions) Apply(action);
}

bool World::Solved() const { return map.Wrapped(); }
