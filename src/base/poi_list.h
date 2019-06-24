#pragma once

#include "base/map.h"
#include "base/poi.h"
#include <vector>

class POIList {
 public:
  std::vector<POI> items;

  void Update(const Map& map);
};
