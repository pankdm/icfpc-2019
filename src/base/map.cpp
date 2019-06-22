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
  auto vst = Split(desc, ',');
  std::vector<std::string> vs;
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
  std::vector<std::vector<int>> vvy(xsize);
  for (size_t i = 1; i < v.size(); ++i) {
    if (v[i - 1].y == v[i].y) {
      int x1 = std::min(v[i - 1].x, v[i].x);
      int x2 = std::max(v[i - 1].x, v[i].x);
      for (int x = x1; x < x2; ++x) {
        vvy[x].push_back(v[i].y);
      }
    } else {
      assert(v[i - 1].x == v[i].x);
    }
  }
  for (int x = 0; x < xsize; ++x) {
    auto& vy = vvy[x];
    std::sort(vy.begin(), vy.end());
    assert((vy.size() % 2) == 0);
    vy.push_back(maxy);
    vy.push_back(maxy);
    int y = 0;
    for (unsigned i = 0; i < vy.size(); i += 2) {
      for (; y < vy[i]; ++y) {
        Get(x, y).SetBlock();
      }
      y = vy[i + 1];
    }
  }
}

int Map::Size() const { return xsize * ysize; }

int Map::X(int index) const { return index / ysize; }
int Map::Y(int index) const { return index % ysize; }

int Map::Index(int x, int y) const { return x * ysize + y; }

bool Map::Inside(int x, int y) const {
  return (x >= 0) && (x < xsize) && (y >= 0) && (y < ysize);
}

Square& Map::operator[](int index) { return map[index]; }
const Square& Map::operator[](int index) const { return map[index]; }

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
  if (Inside(x, y)) {
    int index = Index(x, y);
    Square& s = map[index];
    if (!s.WrappedOrBlocked()) {
      s.Wrap();
      if (save_wraps) wraps_history.push(index);
    }
  }
}

bool Map::HasBeacon(int x, int y) const {
  assert(Inside(x, y));
  return Get(x, y).CheckItem() == Item::BEACON;
}

bool Map::HasExtension(int index) const {
  return HasExtension(X(index), Y(index));
}

bool Map::HasExtension(int x, int y) const {
  assert(Inside(x, y));
  return Get(x, y).CheckItem() == Item::EXTENSION;
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
