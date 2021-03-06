#include "base/boosters_queue.h"

#include "common/always_assert.h"

void BoostersQueue::Add(const BoosterTime& tnow) { q.push(tnow); }

bool BoostersQueue::Available(const BoosterTime& tnow) const {
  return (!locked) && !q.empty() && q.front().ValidToUse(tnow);
}

bool BoostersQueue::AvailableOrLocked(const BoosterTime& tnow) const {
  return locked || (!q.empty() && q.front().ValidToUse(tnow));
}

void BoostersQueue::LockUntilPicked() { locked = true; }

unsigned BoostersQueue::Size() const { return q.size(); }

void BoostersQueue::Use() {
  ALWAYS_ASSERT(!q.empty());
  q.pop();
  locked = false;
}
