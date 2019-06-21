#pragma once

#include <string>

class Point {
 public:
  int x, y;

 public:
  Point(int _x, int _y);
  Point(const std::string& desc);
};
