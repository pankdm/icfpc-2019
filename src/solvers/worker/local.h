#pragma once

#include "base/action.h"
#include "solvers/worker/base.h"

namespace solvers {
namespace worker {
class Local : public Base {
 protected:
  UnsignedSet target;

 protected:
  void UpdateTarget();

 public:
  void Init(unsigned _windex, unsigned _tindex, WorldTaskSplit& world);
  void ResetTask(unsigned new_index, UnsignedSet& new_task);
  Action NextMove();
};
}  // namespace worker
}  // namespace solvers
