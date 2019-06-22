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
  Direction direction = 0;

 protected:
  void AddManipulatorI(int x, int y);

 public:
  void Init(Boosters& b, Map& map, int _x, int _y);

  void Wrap(Map& map);
  std::vector<std::pair<int, int>> CellsToWrap(Map& map, int dx = 0,
                                               int dy = 0);
  std::vector<std::pair<int, int>> CellsToNewlyWrap(Map& map, int dx = 0,
                                                    int dy = 0);
  void Move(const Direction& d, Map& map, bool drill_enabled);
  void RotateClockwise();
  void RotateCounterClockwise();
  void AddManipulator(const Manipulator& m);
  void PrintNeighborhood(Map& map, int sz);
  std::pair<int, int> GetNextManipulatorPositionNaive(int strategy) const;

  void Apply(unsigned time, Map& map, const Action& action);
};
