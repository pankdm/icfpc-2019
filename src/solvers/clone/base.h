#pragma once

#include "base/action.h"
#include "base/world_task_split.h"

namespace solvers {
namespace clone {
class Base {
 protected:
  WorldTaskSplit* pworld = nullptr;

 public:
  void Init(WorldTaskSplit& world);
  Action NextMove(unsigned worker_index);
  ActionsList NextMove();
};
}  // namespace clone
}  // namespace solvers
