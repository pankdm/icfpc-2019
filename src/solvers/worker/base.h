#pragma once

#include "base/action.h"
#include "base/worker.h"
#include "base/world_task_split.h"
#include "common/unsigned_set.h"

namespace solvers {
namespace worker {
class Base {
 protected:
  unsigned windex;
  WorldTaskSplit* pworld = nullptr;
  UnsignedSet* ptask = nullptr;

 protected:
  Worker& Get();

 public:
  void Init(unsigned _windex, WorldTaskSplit& world, UnsignedSet& task);
  void ResetTask(UnsignedSet& new_task);
  Action NextMove();
};
}  // namespace worker
}  // namespace solvers
