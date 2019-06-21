#include "solutions/test.h"

#include "base/world.h"

namespace solutions {
bool Test(const std::string& task, const ActionsList& actions) {
  World world;
  world.Init(task);
  world.Apply(actions);
  return world.Solved();
}
}  // namespace solutions
