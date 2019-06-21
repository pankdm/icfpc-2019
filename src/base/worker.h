#pragma once

#include "base/direction.h"
#include "base/manipulator.h"
#include "base/map.h"
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
};
