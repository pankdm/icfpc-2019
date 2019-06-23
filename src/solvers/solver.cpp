#include "solvers/solver.h"

#include <cassert>

namespace solvers {
ActionsList Solver::Solve(const std::string& task) {
  assert(false);
  return {};
}

ActionsList Solver::Solve(const std::string& task, const std::string& bonuses) {
  assert(false);
  return {};
}

void Solver::InitBoosters(const std::string& bonuses) {
  world.InitBonuses(bonuses);
}

}  // namespace solvers
