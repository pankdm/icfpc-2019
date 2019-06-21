#pragma once

#include "base/action.h"
#include "base/direction.h"
#include "base/manipulator.h"
#include "base/map.h"
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
  unsigned unused_teleporters = 0;
  unsigned time_fast_wheels = 0;
  unsigned time_drill = 0;

 protected:
  void AddManipulatorI(const Manipulator& m);

 public:
  Worker(Map& map, int _x, int _y);

  void Wrap(Map& map);
  void Move(const Direction& d, Map& map, bool drill_enabled);
  void RotateClockwise();
  void RotateCounterClockwise();
  void AddManipulator(const Manipulator& m);

  void Apply(unsigned time, Map& map, const Action& action);
};
