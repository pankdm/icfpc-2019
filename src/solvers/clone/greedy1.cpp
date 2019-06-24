#include "solvers/clone/greedy1.h"

#include "base/item.h"
#include "base/sleep.h"
#include "solvers/paths/path_to_poi.h"

namespace solvers {
namespace clone {
Action Greedy1::NextMove(unsigned worker_index) {
  ALWAYS_ASSERT(pworld);
  if (worker_index != 0) return ActionType::END;
  if (pworld->GetPOIList(Item::CODEX).items.size() == 0) return ActionType::END;
  auto& w = pworld->GetWorker(worker_index);
  const auto& poi_list = pworld->GetPOIList(Item::CLONE);
  if (!poi_list.items.empty()) return PathToPOI(w, *pworld, poi_list);
  if (pworld->boosters.clones.Size() == 0) return ActionType::END;
  auto& map = pworld->map;
  if (map(w.x, w.y).CheckItem() == Item::CODEX) {
    ALWAYS_ASSERT(pworld->boosters.clones.Available(w.Time(pworld->time)));
    return ActionType::CLONE;
  }
  auto a = PathToPOI(w, *pworld, pworld->GetPOIList(Item::CODEX));
  ALWAYS_ASSERT(!Sleep(a));
  return a;
}

ActionsList Greedy1::NextMove() {
  ALWAYS_ASSERT(pworld);
  ActionsList al;
  for (unsigned i = 0; i < pworld->WCount(); ++i) al.emplace_back(NextMove(i));
  return al;
}
}  // namespace clone
}  // namespace solvers
