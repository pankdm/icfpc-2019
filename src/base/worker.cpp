#include "base/worker.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/item.h"
#include "base/manipulator.h"
#include "base/map.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>

void Worker::AddManipulatorI(int x, int y) {
  manipulators.emplace_back(Manipulator(x, y));
}

void Worker::Init(Boosters& b, Map& map, int _x, int _y) {
  pboosters = &b;
  x = _x;
  y = _y;
  AddManipulatorI(0, 0);
  AddManipulatorI(1, 1);
  AddManipulatorI(1, 0);
  AddManipulatorI(1, -1);
  Wrap(map);
}

std::pair<int, int> Worker::GetNextManipulatorPositionNaive() const {
  assert(unused_extensions);
  std::set<std::pair<int, int>> ms;
  for (const Manipulator& cm : manipulators) {
    ms.insert(std::make_pair(cm.X(), cm.Y()));
  }
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if ((i == 0 && j == 0) || i * j != 0) {
        continue;
      }
      if (ms.find(std::make_pair(i, j)) == ms.end()) {
        continue;
      }
      if (i == 0) {
        while (true) {
          for (int sign = -1; sign <= 1; sign += 2) {
            auto p = std::make_pair(i * sign, j);
            if (ms.count(p) == 0) {
              return p;
            }
          }
          i++;
        }
      } else if (j == 0) {
        while (true) {
          for (int sign = -1; sign <= 1; sign += 2) {
            auto p = std::make_pair(i, j * sign);
            if (ms.count(p) == 0) {
              return p;
            }
          }
          j++;
        }
      } else {
        assert(false);
      }
    }
  }
  assert(false);
  return {0, 0};
}

// What cells will be wrapped if we move by dx, dy?
std::vector<std::pair<int, int>> Worker::CellsToWrap(Map& map, int dx, int dy) {
  std::vector<std::pair<int, int>> cells;
  for (Manipulator& m : manipulators) {
    bool ok = true;
    for (auto xy : m.CellsToCheck()) {
      if (!map.Inside(xy.first + x, xy.second + y)) {
        ok = false;
        break;
      }
      if (map.Get(xy.first + x, xy.second + y).Blocked()) {
        ok = false;
        break;
      }
    }
    if (ok) {
      cells.push_back(std::make_pair(x + m.X(), y + m.Y()));
    }
  }
  return cells;
}

void Worker::Wrap(Map& map) {
  for (auto p : CellsToWrap(map)) {
    map.Wrap(p.first, p.second);
  }
}

void Worker::Move(const Direction& d, Map& map, bool drill_enabled) {
  x += d.DX();
  y += d.DY();
  assert(map.ValidToMove(x, y, drill_enabled));
  if (drill_enabled) map.Drill(x, y);
  Item item = map(x, y).GetItem();
  switch (item) {
    case Item::EXTENSION:
      pboosters->unused_extensions += 1;
      break;
    case Item::FAST_WHEELS:
      pboosters->unused_fast_wheels += 1;
      break;
    case Item::DRILL:
      pboosters->unused_drills += 1;
      break;
    case Item::TELEPORT:
      pboosters->unused_teleporters += 1;
      break;
    case Item::CLONE:
      pboosters->unused_clones += 1;
      break;
    case Item::NONE:
    case Item::BEACON:
    case Item::CODEX:
      break;
    default:
      assert(false);
  }
}

void Worker::RotateClockwise() {
  for (Manipulator& m : manipulators) m.RotateClockwise();
  direction = Direction((direction.direction + 3) % 4);
}

void Worker::RotateCounterClockwise() {
  for (Manipulator& m : manipulators) m.RotateCounterClockwise();
  direction = Direction((direction.direction + 1) % 4);
}

void Worker::AddManipulator(const Manipulator& m) {
  assert(pboosters->unused_extensions > 0);
  bool valid = false;
  for (const Manipulator& cm : manipulators) {
    if (std::abs(cm.X() - m.X()) + std::abs(cm.Y() - m.Y()) == 1) {
      valid = true;
      break;
    }
  }
  assert(valid);
  pboosters->unused_extensions -= 1;
  AddManipulatorI(m.X(), m.Y());
}

void Worker::Apply(unsigned time, Map& map, const Action& action) {
  switch (action.type) {
    case ActionType::MOVE_RIGHT:
    case ActionType::MOVE_UP:
    case ActionType::MOVE_LEFT:
    case ActionType::MOVE_DOWN: {
      bool drill_enabled = (time_drill >= time);
      Direction d(action.type);
      Move(d, map, drill_enabled);
      Wrap(map);
      if (time_fast_wheels >= time) {
        if (map.ValidToMove(x + d.DX(), y + d.DY(), drill_enabled))
          Move(d, map, drill_enabled);
        Wrap(map);
      }
    } break;
    case ActionType::DO_NOTHING:
      break;
    case ActionType::ROTATE_CLOCKWISE:
      RotateClockwise();
      Wrap(map);
      break;
    case ActionType::ROTATE_COUNTERCLOCKWISE:
      RotateCounterClockwise();
      Wrap(map);
      break;
    case ActionType::ATTACH_MANIPULATOR:
      AddManipulator({action.x, action.y});
      Wrap(map);
      break;
    case ActionType::ATTACH_FAST_WHEELS:
      assert(pboosters->unused_fast_wheels > 0);
      pboosters->unused_fast_wheels -= 1;
      time_fast_wheels = time + TIME_FAST_WHEELS + 1;
      break;
    case ActionType::USING_DRILL:
      assert(pboosters->unused_fast_wheels > 0);
      pboosters->unused_fast_wheels -= 1;
      time_drill = time + TIME_DRILL + 1;
      break;
    case ActionType::SET_BEACON:
      assert(pboosters->unused_teleporters > 0);
      pboosters->unused_teleporters -= 1;
      map.SetBeacon(x, y);
      break;
    case ActionType::SHIFT:
      assert(map.HasBeacon(action.x, action.y));
      x = action.x;
      y = action.y;
      Wrap(map);
      break;
    case ActionType::CLONE:
      assert(false);
      break;
    default:
      assert(false);
  }
}
