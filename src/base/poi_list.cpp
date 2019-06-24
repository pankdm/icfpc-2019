#include "base/poi_list.h"

#include <algorithm>

void POIList::Update(const Map& map) {
  for (unsigned i = 0; i < items.size();) {
    if (map[items[i].location].CheckItem() == items[i].item) {
      ++i;
    } else {
      if (i + 1 < items.size()) std::swap(items[i], items.back());
      items.pop_back();
    }
  }
}
