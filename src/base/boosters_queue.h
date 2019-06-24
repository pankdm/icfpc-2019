#pragma once

#include "base/booster_time.h"
#include <queue>

class BoostersQueue {
 protected:
  std::queue<BoosterTime> q;
  bool locked{};

 public:
  void Add(const BoosterTime& tnow);
  bool Available(const BoosterTime& tnow) const;
  bool AvailableOrLocked(const BoosterTime& tnow) const;
  void LockUntilPicked();
  unsigned Size() const;
  void Use();
};
