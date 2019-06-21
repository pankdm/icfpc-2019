#pragma once

#include "base/action.h"
#include "base/action_type.h"
#include "base/direction.h"
#include "base/manipulator.h"
#include "base/map.h"
#include <cassert>
#include <vector>

class Worker {
 public:
  int x;
  int y;

  std::vector<Manipulator> manipulators;

 public:
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
