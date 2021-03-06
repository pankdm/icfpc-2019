#include "solvers/worker/local.h"

#include "solvers/paths/path_to_target.h"
#include "common/always_assert.h"
#include <algorithm>

namespace solvers {
namespace worker {
void Local::UpdateTarget() {
  unsigned min_ds_size = target.TotalSize();
  const auto& task = pworld->GetTask(tindex);
  for (unsigned u : task.List()) {
    min_ds_size = std::min(min_ds_size, pworld->DSSize(u));
  }
  target.Clear();
  for (unsigned u : task.List()) {
    if (pworld->DSSize(u) == min_ds_size) target.Insert(u);
  }
}

void Local::Init(unsigned _windex, unsigned _tindex, WorldTaskSplit& world) {
  Base::Init(_windex, _tindex, world);
  target.Resize(world.Size());
  UpdateTarget();
}

void Local::ResetTask(unsigned new_index) {
  Base::ResetTask(new_index);
  UpdateTarget();
}

Action Local::NextMove() {
  ALWAYS_ASSERT(pworld);
  const auto& task = pworld->GetTask(tindex);
  if (task.Empty()) return ActionType::DO_NOTHING;
  if (pworld->UpdateTaskRequired(tindex)) {
    pworld->UpdateTask(tindex);
    UpdateTarget();
  }
  return PathToTarget(Base::Get(), *pworld, target);
}
}  // namespace worker
}  // namespace solvers
