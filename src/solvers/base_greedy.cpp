#include "solvers/base_greedy.h"

#include "base/worker.h"
#include "solvers/paths/path_to_target.h"

namespace solvers {
ActionsList BaseGreedy::Solve(const std::string& task) {
  world.Init(task);
  auto& w = world.GetWorker();
  ActionsList actions;
  for (; !world.Solved();) {
    Action a = PathToTarget(w, world, world.UnwrappedSet());
    actions.push_back(a);
    world.Apply(a);
  }
  return actions;
}
}  // namespace solvers
