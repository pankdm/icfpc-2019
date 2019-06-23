#pragma once

#include "base/booster_time.h"
#include <queue>

class BoostersQueue {
 protected:
  std::queue<BoosterTime> q;

 public:
  void Add(const BoosterTime& tnow);
  bool Available(const BoosterTime& tnow) const;
  unsigned Size() const;
  void Use();
};
