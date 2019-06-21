#pragma once

#include "base/world.h"
#include "solutions/solution.h"

namespace solutions {
class BaseGreedy : public Solution {
 protected:
  World world;

 public:
  ActionsList Solve(const std::string& task);
};
}  // namespace solutions
