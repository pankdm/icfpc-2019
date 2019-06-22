#include "solvers/base_greedy3.h"

namespace solvers {

Action BaseGreedy3::NextMove() {
  if (world.boosters.unused_extensions) {
    auto p = world.worker.GetNextManipulatorPositionNaive();
    Action a(ActionType::ATTACH_MANIPULATOR, p.first, p.second);
    world.Apply(a);
    Update();
    return a;
  }
  return BaseGreedy2::NextMove();
}

ActionsList BaseGreedy3::Solve(const std::string& task) {
  Init(task);
  ActionsList actions;
  for (; !Wrapped();) {
    Action a = NextMove();
    if (a.type == ActionType::END) return actions;
    actions.emplace_back(a);
  }
  return actions;
}
}  // namespace solvers
