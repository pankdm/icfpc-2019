#include "solvers/worker/greedy.h"

#include "solvers/paths/path_to_target.h"
#include "common/always_assert.h"

namespace solvers {
namespace worker {
Action Greedy::NextMove() {
  ALWAYS_ASSERT(Base::pworld && Base::ptask);
  return PathToTarget(Base::Get(), *Base::pworld, *Base::ptask);
}
}  // namespace worker
}  // namespace solvers
