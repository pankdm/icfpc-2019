#pragma once

#include "base/action.h"
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
  unsigned unused_extensions = 0;
  unsigned unused_fast_wheels = 0;
  unsigned unused_drills = 0;
  unsigned unused_teleporters = 0;
  unsigned unused_clones = 0;
  unsigned time_fast_wheels = 0;
  unsigned time_drill = 0;

 protected:
  void AddManipulatorI(int x, int y);

 public:
  void Init(Map& map, int _x, int _y);

  void Wrap(Map& map);
  void Move(const Direction& d, Map& map, bool drill_enabled);
  void RotateClockwise();
  void RotateCounterClockwise();
  void AddManipulator(const Manipulator& m);
  std::pair<int, int> GetNextManipulatorPositionNaive() const;

  void Apply(unsigned time, Map& map, const Action& action);
};
