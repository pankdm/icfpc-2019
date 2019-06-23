#pragma once

#include "base/item.h"
#include "base/square.h"
#include "map"
#include "set"
#include <queue>
#include <string>
#include <vector>

class Map {
 public:
  int xsize, ysize;
  std::vector<Square> map;

 public:
  // For solvers
  bool save_wraps = false;
  std::queue<int> wraps_history;
  std::map<Item, std::set<std::pair<int, int>>> items_coords;

 protected:
  void Resize(int _xsize, int _ysize);
  void AddBlock(int x0, int x1, int y0, int y1);

 public:
  Map();
  Map(const std::string& desc);

  void AddBlock(const std::string& desc);
  void Init(const std::string& desc);

  int Size() const;
  int Index(int x, int y) const;
  Square& operator[](int index);
  const Square& operator[](int index) const;

  bool Inside(int x, int y) const;
  int X(int index) const;
  int Y(int index) const;
  bool HasExtension(int index) const;
  bool HasExtension(int x, int y) const;
  Square& Get(int x, int y);
  const Square& Get(int x, int y) const;
  Square& operator()(int x, int y);
  const Square& operator()(int x, int y) const;

  bool ValidToMove(int x, int y, bool drill_enabled = false) const;
  void Drill(int x, int y);
  void Wrap(int x, int y);
  bool HasBeacon(int x, int y) const;
  void SetBeacon(int x, int y);

  unsigned Count(Item item) const;

  bool Wrapped() const;
  void Print() const;
};
