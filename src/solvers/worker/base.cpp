#include "solvers/worker/base.h"

namespace solvers {
namespace worker {
Worker& Base::Get() { return pworld->GetWorker(windex); }

void Base::Init(unsigned _windex, unsigned _tindex, WorldTaskSplit& world) {
  windex = _windex;
  tindex = _tindex;
  pworld = &world;
  Get().task_assigned = true;
}

void Base::ResetTask(unsigned new_index) { tindex = new_index; }

Action Base::NextMove() { return ActionType::DO_NOTHING; }
}  // namespace worker
}  // namespace solvers
