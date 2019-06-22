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

void Worker::Init(Map& map, int _x, int _y) {
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
}
// TODO:
//   Add reachability check.
void Worker::Wrap(Map& map) {
  for (Manipulator& m : manipulators) {
    bool ok = true;
    for (auto xy : m.CellsToCheck()) {
      if (map.Get(xy.first + x, xy.second + y).Blocked()) {
        ok = false;
        break;
      }
    }
    if (ok) {
      map.Wrap(x + m.X(), y + m.Y());
    } else {
      map.Print();
      std::cout << x << " " << y << std::endl;
    }
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
      ++unused_extensions;
      break;
    case Item::FAST_WHEELS:
      ++unused_fast_wheels;
      break;
    case Item::DRILL:
      ++unused_drills;
      break;
    case Item::TELEPORT:
      ++unused_teleporters;
      break;
    case Item::CLONE:
      ++unused_clones;
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
}

void Worker::RotateCounterClockwise() {
  for (Manipulator& m : manipulators) m.RotateCounterClockwise();
}

void Worker::AddManipulator(const Manipulator& m) {
  assert(unused_extensions > 0);
  bool valid = false;
  for (const Manipulator& cm : manipulators) {
    if (std::abs(cm.X() - m.X()) + std::abs(cm.Y() - m.Y()) == 1) {
      valid = true;
      break;
    }
  }
  assert(valid);
  --unused_extensions;
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
      assert(unused_fast_wheels > 0);
      --unused_fast_wheels;
      time_fast_wheels = time + TIME_FAST_WHEELS + 1;
      break;
    case ActionType::USING_DRILL:
      assert(unused_fast_wheels > 0);
      --unused_fast_wheels;
      time_drill = time + TIME_DRILL + 1;
      break;
    case ActionType::SET_BEACON:
      assert(unused_teleporters > 0);
      --unused_teleporters;
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
