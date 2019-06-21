#include "base/map.h"

#include "base/point.h"
#include "base/square.h"
#include "utils/split.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

void Map::Resize(int _xsize, int _ysize) {
  xsize = _xsize;
  ysize = _ysize;
  map.resize(xsize * ysize);
}

void Map::AddBlock(int x0, int x1, int y0, int y1) {
  for (int x = x0; x < x1; ++x) {
    for (int y = y0; y < y1; ++y) Get(x, y).SetBlock();
  }
}

Map::Map() : xsize(0), ysize(0) {}

Map::Map(const std::string& desc) { Init(desc); }

void Map::AddBlock(const std::string& desc) {
  Map t(desc);
  for (int x = 0; x < t.xsize; ++x) {
    for (int y = 0; y < t.ysize; ++y) {
      if (!t.Get(x, y).Blocked()) Get(x, y).SetBlock();
    }
  }
}

void Map::Init(const std::string& desc) {
  int maxx = 0, maxy = 0;
  std::vector<Point> v;
  std::vector<std::string> vst = Split(desc, ','), vs;
  for (unsigned i = 0; i < vst.size(); i += 2)
    vs.emplace_back(vst[i] + "," + vst[i + 1]);
  for (const std::string& st : vs) {
    Point p(st);
    maxx = std::max(p.x, maxx);
    maxy = std::max(p.y, maxy);
    v.emplace_back(p);
  }
  assert(v.size() > 1);
  v.push_back(v[0]);
  Resize(maxx, maxy);
  for (size_t i = 1; i < v.size(); ++i) {
    if (v[i - 1].x == v[i].x) {
      assert(v[i - 1].y != v[i].y);
      if (v[i - 1].y < v[i].y) {
        AddBlock(v[i].x, maxx, v[i - 1].y, v[i].y);
      } else {
        AddBlock(0, v[i].x, v[i].y, v[i - 1].y);
      }
    } else if (v[i - 1].y == v[i].y) {
      assert(v[i - 1].x != v[i].x);
      if (v[i - 1].x < v[i].x) {
        AddBlock(v[i - 1].x, v[i].x, 0, v[i].y);
      } else {
        AddBlock(v[i].x, v[i - 1].x, v[i].y, maxy);
      }
    } else {
      assert(false);
    }
  }
}

bool Map::Inside(int x, int y) const {
  return (x >= 0) && (x < xsize) && (y >= 0) && (y < ysize);
}

Square& Map::Get(int x, int y) { return map[x * ysize + y]; }
const Square& Map::Get(int x, int y) const { return map[x * ysize + y]; }

Square& Map::operator()(int x, int y) { return Get(x, y); }
const Square& Map::operator()(int x, int y) const { return Get(x, y); }

bool Map::ValidToMove(int x, int y, bool drill_enabled) const {
  return Inside(x, y) && (drill_enabled || !Get(x, y).Blocked());
}

void Map::Drill(int x, int y) {
  assert(Inside(x, y));
  Get(x, y).Drill();
}

void Map::Wrap(int x, int y) {
  if (Inside(x, y)) Get(x, y).Wrap();
}

bool Map::HasBeacon(int x, int y) const {
  assert(Inside(x, y));
  return Get(x, y).CheckItem() == Item::BEACON;
}

void Map::SetBeacon(int x, int y) {
  assert(Inside(x, y));
  Get(x, y).SetBeacon();
}

bool Map::Wrapped() const {
  for (const Square& square : map) {
    if (!square.WrappedOrBlocked()) return false;
  }
  return true;
}

void Map::Print() const {
  for (int y = ysize - 1; y >= 0; --y) {
    for (int x = 0; x < xsize; ++x) {
      auto& s = Get(x, y);
      if (s.Blocked())
        std::cout << "#";
      else if (s.Wrapped())
        std::cout << "~";
      else
        std::cout << ".";
    }
    std::cout << std::endl;
  }
}
