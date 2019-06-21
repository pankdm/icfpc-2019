#include "base/worker.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/item.h"
#include "base/manipulator.h"
#include "base/map.h"
#include <cassert>
#include <cmath>

void Worker::AddManipulatorI(const Manipulator& m) {
  manipulators.emplace_back(m);
}

void Worker::Init(Map& map, int _x, int _y) {
  x = _x;
  y = _y;
  AddManipulatorI({0, 0});
  AddManipulatorI({1, 1});
  AddManipulatorI({1, 0});
  AddManipulatorI({1, -1});
  Wrap(map);
}

// TODO:
//   Add reachability check.
void Worker::Wrap(Map& map) {
  for (Manipulator& m : manipulators) {
    map.Wrap(x + m.x, y + m.y);
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
    if (std::abs(cm.x - m.x) + std::abs(cm.y - m.y) == 1) {
      valid = true;
      break;
    }
  }
  assert(valid);
  --unused_extensions;
  AddManipulatorI(m);
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
    default:
      assert(false);
  }
}
