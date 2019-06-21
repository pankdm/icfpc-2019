#pragma once

#include <cassert>
#include <vector>

class Direction {
 public:
  unsigned direction;

 public:
  char ToLetter() const {
    static const std::vector<char> v{'D', 'W', 'A', 'S'};
    return v[direction];
  }

  void FromLetter(char c) {
    switch (c) {
      case 'D':
        direction = 0;
        break;
      case 'W':
        direction = 1;
        break;
      case 'A':
        direction = 2;
        break;
      case 'S':
        direction = 3;
        break;
      default:
        assert(false);
    }
  }

  int DX() const {
    static const std::vector<int> dx{-1, 0, 1, 0};
    return dx[direction];
  }

  int DY() const {
    static const std::vector<int> dy{0, 1, 0, -1};
    return dy[direction];
  }
};
