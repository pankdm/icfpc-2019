#pragma once

#include "base/action.h"
#include "base/world_ext.h"
#include <string>

namespace solvers {
class Solver {
 public:
  ActionsList Solve(const std::string& task);
  ActionsList Solve(const std::string& task, const std::string& bonuses);
  void InitBoosters(const std::string& bonuses);

 protected:
  WorldExt world;
};
}  // namespace solvers
