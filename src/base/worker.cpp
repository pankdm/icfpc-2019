#include "base/worker.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/item.h"
#include "base/manipulator.h"
#include "base/map.h"
#include "common/always_assert.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>

void Worker::AddManipulatorI(int x, int y) {
  manipulators.emplace_back(Manipulator(x, y));
}

void Worker::Init(Boosters& b, Map& map, int _x, int _y, unsigned _index) {
  pboosters = &b;
  x = _x;
  y = _y;
  index = _index;
  AddManipulatorI(0, 0);
  AddManipulatorI(1, 1);
  AddManipulatorI(1, 0);
  AddManipulatorI(1, -1);
  Wrap(map);
}

BoosterTime Worker::Time(unsigned time) const { return {time, index}; }

std::pair<int, int> Worker::GetNextManipulatorPositionNaive(
    int strategy) const {
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
      if (strategy == 1) {
        int ni = 0;
        int nj = 0;
        while (true) {
          auto p = std::make_pair(ni, nj);
          if (ms.count(p) == 0) {
            return p;
          }
          ni = ni + i;
          nj = nj + j;
        }
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
      }
      if (j == 0) {
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
        ALWAYS_ASSERT(false);
      }
    }
  }
  ALWAYS_ASSERT(false);
  return {0, 0};
}

// What cells will be wrapped if we move by dx, dy?
std::vector<std::pair<int, int>> Worker::CellsToWrap(Map& map, int dx, int dy) {
  std::vector<std::pair<int, int>> cells;
  for (Manipulator& m : manipulators) {
    if (!map.Inside(x + dx + m.X(), y + dy + m.Y())) {
      continue;
    }
    bool ok = true;
    for (auto xy : m.CellsToCheck()) {
      if (map.Get(xy.first + x + dx, xy.second + y + dy).Blocked()) {
        ok = false;
        break;
      }
    }
    if (ok) {
      cells.push_back(std::make_pair(x + dx + m.X(), y + dy + m.Y()));
    }
  }
  return cells;
}

void Worker::PrintNeighborhood(Map& map, int sz) {
  std::set<std::pair<int, int>> ms;
  for (auto p : CellsToWrap(map)) {
    ms.insert(p);
  }
  for (int j = y + sz; j >= y - sz; --j) {
    for (int i = x - sz; i <= x + sz; ++i) {
      if (!map.Inside(i, j)) {
        continue;
      }
      auto& s = map.Get(i, j);
      if (i == x && j == y)
        std::cout << "X";
      else if (s.Blocked())
        std::cout << "#";
      else if (ms.count(std::make_pair(i, j)) != 0)
        std::cout << "H";
      else if (s.Wrapped())
        std::cout << "~";
      else if (map.HasExtension(i, j))
        std::cout << "!";
      else
        std::cout << ".";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

std::vector<std::pair<int, int>> Worker::CellsToNewlyWrap(Map& map, int dx,
                                                          int dy) {
  std::vector<std::pair<int, int>> cells;
  for (auto p : CellsToWrap(map, dx, dy)) {
    if (!map.Get(p.first, p.second).WrappedOrBlocked()) {
      cells.push_back(p);
    }
  }
  return cells;
}

void Worker::Wrap(Map& map) {
  for (auto p : CellsToWrap(map)) {
    map.Wrap(p.first, p.second);
  }
}

void Worker::Move(const Direction& d, Map& map, unsigned time,
                  bool drill_enabled) {
  x += d.DX();
  y += d.DY();
  ALWAYS_ASSERT(map.ValidToMove(x, y, drill_enabled));
  if (drill_enabled) map.Drill(x, y);
  Item item = map(x, y).GetItem();
  if ((item != Item::BEACON) && (item != Item::CODEX) && (item != Item::NONE)) {
    map.items_coords[item].erase(std::make_pair(x, y));
  }
  switch (item) {
    case Item::EXTENSION:
      pboosters->extensions.Add(Time(time));
      break;
    case Item::FAST_WHEELS:
      pboosters->fast_wheels.Add(Time(time));
      break;
    case Item::DRILL:
      pboosters->drills.Add(Time(time));
      break;
    case Item::TELEPORT:
      pboosters->teleporters.Add(Time(time));
      break;
    case Item::CLONE:
      pboosters->clones.Add(Time(time));
      break;
    case Item::NONE:
    case Item::BEACON:
    case Item::CODEX:
      break;
    default:
      ALWAYS_ASSERT(false);
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

void Worker::AddManipulator(const Manipulator& m, unsigned time) {
  ALWAYS_ASSERT(pboosters->extensions.AvailableOrLocked(Time(time)));
  bool valid = false;
  for (const Manipulator& cm : manipulators) {
    if (std::abs(cm.X() - m.X()) + std::abs(cm.Y() - m.Y()) == 1) {
      valid = true;
      break;
    }
  }
  ALWAYS_ASSERT(valid);
  pboosters->extensions.Use();
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
      if (!map.ValidToMove(x + d.DX(), y + d.DY())) {
        valid = false;
      }
      Move(d, map, time, drill_enabled);
      Wrap(map);
      if (time_fast_wheels >= time) {
        if (map.ValidToMove(x + d.DX(), y + d.DY(), drill_enabled)) {
          Move(d, map, time, drill_enabled);
          Wrap(map);
        }
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
      AddManipulator({action.x, action.y}, time);
      Wrap(map);
      break;
    case ActionType::ATTACH_FAST_WHEELS:
      ALWAYS_ASSERT(pboosters->fast_wheels.Available(Time(time)));
      pboosters->fast_wheels.Use();
      time_fast_wheels = time + TIME_FAST_WHEELS;
      break;
    case ActionType::USING_DRILL:
      ALWAYS_ASSERT(pboosters->fast_wheels.Available(Time(time)));
      pboosters->fast_wheels.Use();
      time_drill = time + TIME_DRILL + 1;
      break;
    case ActionType::SET_BEACON:
      ALWAYS_ASSERT(pboosters->teleporters.Available(Time(time)));
      pboosters->teleporters.Use();
      map.SetBeacon(x, y);
      break;
    case ActionType::SHIFT:
      ALWAYS_ASSERT(map.HasBeacon(action.x, action.y));
      x = action.x;
      y = action.y;
      Wrap(map);
      break;
    case ActionType::CLONE:
      ALWAYS_ASSERT(false);
      break;
    default:
      ALWAYS_ASSERT(false);
  }
  if (!map.ValidToMove(x, y)) {
    valid = false;
  }
}
