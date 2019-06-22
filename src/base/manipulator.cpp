#include "base/manipulator.h"
#include "iostream"
#include <cmath>

Manipulator::Manipulator(int _x, int _y) {
  x = _x;
  y = _y;

  for (int i = 0; std::abs(i) <= std::abs(x); i += (x > 0 ? 1 : -1)) {
    for (int j = 0; std::abs(j) <= std::abs(y); j += (y > 0 ? 1 : -1)) {
      if ((i == 0 && j == 0) || (i == x && j == y)) {
        continue;
      }
      bool pos = false, neg = false;
      for (int a = 2 * i - 1; a <= 2 * i + 1; a += 2) {
        for (int b = 2 * j - 1; b <= 2 * j + 1; b += 2) {
          int vp = x * b - y * a;
          if (vp > 0) {
            pos = true;
          }
          if (vp < 0) {
            neg = true;
          }
        }
      }
      if (pos && neg) {
        to_check.push_back(std::make_pair(i, j));
      }
    }
  }
}

std::vector<std::pair<int, int>> Manipulator::CellsToCheck() {
  return to_check;
}

void Manipulator::RotateClockwise() {
  int t = x;
  x = -y;
  y = t;
  auto new_to_check = to_check;
  new_to_check.clear();
  for (auto p : to_check) {
    new_to_check.push_back(std::make_pair(-p.second, p.first));
  }
  to_check = new_to_check;
}

int Manipulator::X() const { return x; }
int Manipulator::Y() const { return y; }

void Manipulator::RotateCounterClockwise() {
  int t = x;
  x = y;
  y = -t;
  auto new_to_check = to_check;
  new_to_check.clear();
  for (auto p : to_check) {
    new_to_check.push_back(std::make_pair(p.second, -p.first));
  }
  to_check = new_to_check;
}
