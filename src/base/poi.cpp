#include "base/poi.h"

#include "common/graph/graph/distance.h"

POI::POI(Item _item, unsigned index, const UndirectedGraph& g) {
  item = _item;
  location = index;
  vd = DistanceFromSource(g, index);
}
