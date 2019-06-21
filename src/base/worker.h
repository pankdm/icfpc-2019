#pragma once

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/manipulator.h"
#include "base/map.h"
#include <cassert>
#include <cmath>
#include <vector>

// TODO:
//  Support boosters (get from map and apply).
class Worker {
 public:
  int x;
  int y;

  std::vector<Manipulator> manipulators;

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

  void Move(const Direction& d, Map& map) {
    x += d.DX();
    y += d.DY();
    assert(map.ValidToMove(x, y));
  }

  void RotateClockwise() {
    for (Manipulator& m : manipulators) m.RotateClockwise();
  }

  void RotateCounterClockwise() {
    for (Manipulator& m : manipulators) m.RotateCounterClockwise();
  }

  void AddManipulator(const Manipulator& m) {
    bool valid = false;
    for (const Manipulator& cm : manipulators) {
      if (std::abs(cm.x - m.x) + std::abs(cm.y - m.y) == 1) {
        valid = true;
        break;
      }
    }
    assert(valid);
    AddManipulatorI(m);
  }

  void Apply(Map& map, const Action& action) {
    switch (action.type) {
      case ActionType::MOVE_RIGHT:
      case ActionType::MOVE_UP:
      case ActionType::MOVE_LEFT:
      case ActionType::MOVE_DOWN:
        Direction d(action.type);
        Move(d, map);
        Wrap(map);
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
      default:
        assert(false);
    }
  }
};
