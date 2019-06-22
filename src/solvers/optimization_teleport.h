#pragma once

#include "base/action.h"
#include "base/world.h"

namespace solvers {

class TeleportOptimization {
public:
  ActionsList apply(const std::string& task, const ActionsList& actions);
};

}  // namespace solvers
