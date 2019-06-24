#pragma once

#include "base/action.h"
#include "base/world_ext.h"

namespace solvers {
namespace clone {
class Base {
 protected:
  WorldExt* pworld = nullptr;

 public:
  void Init(WorldExt& world);
  Action NextMove(unsigned worker_index);
  ActionsList NextMove();
};
}  // namespace clone
}  // namespace solvers
