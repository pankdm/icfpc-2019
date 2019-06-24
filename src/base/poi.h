#pragma once

#include "base/item.h"
#include "common/graph/graph.h"
#include <vector>

class POI {
 public:
  Item item;
  unsigned location;
  std::vector<unsigned> vd;

  POI(Item _item, unsigned index, const UndirectedGraph& g);
};
