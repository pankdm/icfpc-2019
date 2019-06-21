#pragma once

#include "base/square.h"
#include <vector>

class Map {
 public:
  int xsize, ysize;
  std::vector<Square> map;

 public:
  Map(int _xsize, int _ysize) : xsize(_xsize), ysize(_ysize) {
    map.resize(xsize * ysize);
  }

  bool Inside(int x, int y) const {
    return (x >= 0) && (x < xsize) && (y >= 0) && (y < ysize);
  }

  Square& operator()(int x, int y) { return map[x * ysize + y]; }
  const Square& operator()(int x, int y) const { return map[x * ysize + y]; }

  bool ValidToMove(int x, int y) const {
    return Inside(x, y) && !operator()(x, y).Blocked();
  }

  void Wrap(int x, int y) {
    if (Inside(x, y)) operator()(x, y).Wrap();
  }
};
