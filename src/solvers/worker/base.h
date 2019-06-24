#pragma once

#include "base/action.h"
#include "base/worker.h"
#include "base/world_task_split.h"
#include "common/unsigned_set.h"

namespace solvers {
namespace worker {
class Base {
 public:
  unsigned windex;
  unsigned tindex;
  WorldTaskSplit* pworld = nullptr;

 protected:
  Worker& Get();

 public:
  void Init(unsigned _windex, unsigned _tindex, WorldTaskSplit& world);
  void ResetTask(unsigned new_index);
  Action NextMove();
};
}  // namespace worker
}  // namespace solvers
