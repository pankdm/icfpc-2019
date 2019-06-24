#include "solvers/worker/base.h"

namespace solvers {
namespace worker {
Worker& Base::Get() { return pworld->GetWorker(windex); }

void Base::Init(unsigned _windex, WorldTaskSplit& world, UnsignedSet& task) {
  windex = _windex;
  pworld = &world;
  ptask = &task;
  Get().task_assigned = true;
}

void Base::ResetTask(UnsignedSet& new_task) { ptask = &new_task; }

Action Base::NextMove() { return ActionType::DO_NOTHING; }
}  // namespace worker
}  // namespace solvers
