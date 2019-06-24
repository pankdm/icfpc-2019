#include "solvers/clone/base.h"

#include "common/always_assert.h"

namespace solvers {
namespace clone {
void Base::Init(WorldTaskSplit& world) { pworld = &world; }

Action Base::NextMove(unsigned index) { return ActionType::END; }

ActionsList Base::NextMove() {
  ALWAYS_ASSERT(pworld);
  return ActionsList(pworld->WCount(), Action(ActionType::END));
}
}  // namespace clone
}  // namespace solvers
