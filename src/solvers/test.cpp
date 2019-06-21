#include "solvers/test.h"

#include "base/world.h"

namespace solvers {
bool Test(const std::string& task, const ActionsList& actions) {
  World world;
  world.Init(task);
  world.Apply(actions);
  return world.Solved();
}
}  // namespace solvers
