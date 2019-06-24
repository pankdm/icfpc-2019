#pragma once

#include "base/action.h"
#include "base/worker.h"
#include "base/world_ext.h"
#include "common/unsigned_set.h"

namespace solvers {
Action PathToTarget(const Worker& w, const WorldExt& world,
                    const UnsignedSet& target);
}  // namespace solvers
