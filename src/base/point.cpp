#include "base/point.h"

#include <cassert>
#include <string>

Point::Point(int _x, int _y) : x(_x), y(_y) {}

Point::Point(const std::string& desc) {
  assert((desc.size() >= 2) && (desc[0] == '(') && (desc.back() == ')'));
  size_t npos = desc.find(',');
  assert(npos != std::string::npos);
  x = std::stoi(desc.substr(1, npos - 1));
  y = std::stoi(desc.substr(npos + 1, desc.size() - npos - 2));
}

Point Point::operator+(const Direction& d) const {
  return Point(x + d.DX(), y + d.DY());
}
