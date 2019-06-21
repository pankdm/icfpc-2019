#pragma once

class Manipulator {
 public:
  int x, y;

 public:
  void RotateClockwise() {
    int t = x;
    x = -y;
    y = t;
  }

  void RotateCounterClockwise() {
    int t = x;
    x = y;
    y = -t;
  }
};
