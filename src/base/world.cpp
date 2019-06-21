#include "base/world.h"

void World::Apply(const Action& action) { worker.Apply(++time, map, action); }

void World::Apply(const ActionsList& actions) {
  for (const Action& action : actions) Apply(action);
}

bool World::Solved() const { return map.Wrapped(); }
