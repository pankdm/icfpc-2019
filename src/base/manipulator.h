#pragma once

#include <vector>

class Manipulator {
 private:
  int x, y;
  std::vector<std::pair<int, int>> to_check;

 public:
  Manipulator(int _x, int _y);
  void RotateClockwise();
  void RotateCounterClockwise();
  int X() const;
  int Y() const;
  std::vector<std::pair<int, int>> CellsToCheck();
};
