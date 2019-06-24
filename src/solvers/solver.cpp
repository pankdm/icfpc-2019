#include "solvers/solver.h"

#include "common/always_assert.h"

namespace solvers {
ActionsList Solver::Solve(const std::string& task) {
  ALWAYS_ASSERT(false);
  return {};
}

ActionsList Solver::Solve(const std::string& task, const std::string& bonuses) {
  ALWAYS_ASSERT(false);
  return {};
}

void Solver::InitBoosters(const std::string& bonuses) {
  world.InitBonuses(bonuses);
}

}  // namespace solvers
