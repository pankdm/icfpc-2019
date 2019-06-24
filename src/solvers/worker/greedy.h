#pragma once

#include "base/action.h"
#include "solvers/worker/base.h"

namespace solvers {
namespace worker {
class Greedy : public Base {
 public:
  Action NextMove();
};
}  // namespace worker
}  // namespace solvers
