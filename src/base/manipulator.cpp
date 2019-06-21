#include "base/manipulator.h"

void Manipulator::RotateClockwise() {
  int t = x;
  x = -y;
  y = t;
}

void Manipulator::RotateCounterClockwise() {
  int t = x;
  x = y;
  y = -t;
}
