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

void Worker::Init(Boosters& b, Map& map, int _x, int _y) {
  pboosters = &b;
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
}

void Worker::RotateCounterClockwise() {
  for (Manipulator& m : manipulators) m.RotateCounterClockwise();
}

void Worker::AddManipulator(const Manipulator& m) {
  assert(pboosters->unused_extensions > 0);
  bool valid = false;
  for (const Manipulator& cm : manipulators) {
    if (std::abs(cm.x - m.x) + std::abs(cm.y - m.y) == 1) {
      valid = true;
      break;
    }
  }
  assert(valid);
  pboosters->unused_extensions -= 1;
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
