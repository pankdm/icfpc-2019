#pragma once

#include "base/action.h"
#include "solvers/clone/base.h"

namespace solvers {
namespace clone {
// One worker take all clone boosters and apply it
class Greedy1 : public Base {
 public:
  Action NextMove(unsigned worker_index);
  ActionsList NextMove();
};
}  // namespace clone
}  // namespace solvers
