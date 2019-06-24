#pragma once

#include "base/action.h"
#include "base/poi.h"
#include "base/poi_list.h"
#include "base/worker.h"
#include "base/world_ext.h"

namespace solvers {
Action PathToPOI(const Worker& w, const WorldExt& world, const POI& poi);
Action PathToPOI(const Worker& w, const WorldExt& world, const POIList& list);
}  // namespace solvers
