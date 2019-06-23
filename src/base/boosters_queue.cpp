#include "base/boosters_queue.h"

#include <cassert>

void BoostersQueue::Add(const BoosterTime& tnow) { q.push(tnow); }

bool BoostersQueue::Available(const BoosterTime& tnow) const {
  return !q.empty() && q.front().ValidToUse(tnow);
}

unsigned BoostersQueue::Size() const { return q.size(); }

void BoostersQueue::Use() {
  assert(!q.empty());
  q.pop();
}
