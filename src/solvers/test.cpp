#include "solvers/test.h"

#include "base/world.h"

namespace solvers {
unsigned Test(const std::string& task, const ActionsList& actions,
              const std::string& bonuses) {
  if (actions.empty()) return 0;
  World world;
  world.InitBonuses(bonuses);
  world.Init(task);
  world.Apply(actions);
  return world.Solved() ? world.time : 0;
}

unsigned Test(const std::string& task, const ActionsClones& actions,
              const std::string& bonuses) {
  if (actions.empty()) return 0;
  World world;
  world.InitBonuses(bonuses);
  world.Init(task);
  world.ApplyC(actions);
  return world.Solved() ? world.time : 0;
}
}  // namespace solvers
