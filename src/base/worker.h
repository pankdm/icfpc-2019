#pragma once

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/item.h"
#include "base/manipulator.h"
#include "base/map.h"
#include <cassert>
#include <cmath>
#include <vector>

class Worker {
 public:
  int x;
  int y;

  std::vector<Manipulator> manipulators;

  // boosters
  unsigned unused_extensions = 0;
  unsigned unused_fast_wheels = 0;
  unsigned unused_drills = 0;
  unsigned time_fast_wheels = 0;
  unsigned time_drill = 0;

 protected:
  void AddManipulatorI(const Manipulator& m) { manipulators.emplace_back(m); }

 public:
  Worker(Map& map, int _x, int _y) : x(_x), y(_y) {
    AddManipulatorI({0, 0});
    AddManipulatorI({1, 1});
    AddManipulatorI({1, 0});
    AddManipulatorI({1, -1});
    Wrap(map);
  }

  // TODO:
  //   Add reachability check.
  void Wrap(Map& map) {
    for (Manipulator& m : manipulators) {
      map.Wrap(x + m.x, y + m.y);
    }
  }

  void Move(const Direction& d, Map& map, bool drill_enabled = false) {
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
      case Item::NONE:
      case Item::CODEX:
        break;
      default:
        assert(false);
    }
  }

  void RotateClockwise() {
    for (Manipulator& m : manipulators) m.RotateClockwise();
  }

  void RotateCounterClockwise() {
    for (Manipulator& m : manipulators) m.RotateCounterClockwise();
  }

  void AddManipulator(const Manipulator& m) {
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

  void Apply(unsigned time, Map& map, const Action& action) {
    switch (action.type) {
      case ActionType::MOVE_RIGHT:
      case ActionType::MOVE_UP:
      case ActionType::MOVE_LEFT:
      case ActionType::MOVE_DOWN:
        bool drill_enabled = (time_drill >= time);
        Direction d(action.type);
        Move(d, map, drill_enabled);
        Wrap(map);
        if (time_fast_wheels >= time) {
          if (map.ValidToMove(x + d.DX(), y + d.DY(), drill_enabled))
            Move(d, map, drill_enabled);
          Wrap(map);
        }
        break;
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
      default:
        assert(false);
    }
  }
};
