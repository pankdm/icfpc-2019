#pragma once

#include "base/item.h"
#include "base/square.h"
#include <string>
#include <vector>

class Map {
 public:
  int xsize, ysize;
  std::vector<Square> map;

 protected:
  void Resize(int _xsize, int _ysize);
  void AddBlock(int x0, int x1, int y0, int y1);

 public:
  Map();
  Map(const std::string& desc);

  void AddBlock(const std::string& desc);
  void Init(const std::string& desc);

  bool Inside(int x, int y) const;
  Square& Get(int x, int y);
  const Square& Get(int x, int y) const;
  Square& operator()(int x, int y);
  const Square& operator()(int x, int y) const;

  bool ValidToMove(int x, int y, bool drill_enabled = false) const;
  void Drill(int x, int y);
  void Wrap(int x, int y);
  bool HasBeacon(int x, int y) const;
  void SetBeacon(int x, int y);

  bool Wrapped() const;
  void Print() const;
};
