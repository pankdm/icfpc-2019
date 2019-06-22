#pragma once

#include "base/action.h"
#include "base/boosters.h"
#include "base/direction.h"
#include "base/manipulator.h"
#include "base/map.h"
#include <vector>

// TODO:
//   Clones are not supported yet.
class Worker {
 public:
  int x;
  int y;

  std::vector<Manipulator> manipulators;

  // boosters
  Boosters* pboosters = nullptr;
  unsigned time_fast_wheels = 0;
  unsigned time_drill = 0;

 protected:
  void AddManipulatorI(const Manipulator& m);

 public:
  void Init(Boosters& b, Map& map, int _x, int _y);

  void Wrap(Map& map);
  void Move(const Direction& d, Map& map, bool drill_enabled);
  void RotateClockwise();
  void RotateCounterClockwise();
  void AddManipulator(const Manipulator& m);

  void Apply(unsigned time, Map& map, const Action& action);
};
