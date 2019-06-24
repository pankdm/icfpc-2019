#include "solvers/worker/greedy.h"

#include "solvers/paths/path_to_target.h"
#include "common/always_assert.h"

namespace solvers {
namespace worker {
Action Greedy::NextMove() {
  ALWAYS_ASSERT(pworld);
  const auto& task = pworld->GetTask(tindex);
  if (task.Empty())
    return ActionType::DO_NOTHING;
  else
    return PathToTarget(Base::Get(), *pworld, task);
}
}  // namespace worker
}  // namespace solvers
