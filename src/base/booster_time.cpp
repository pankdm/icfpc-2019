#include "base/booster_time.h"

bool BoosterTime::ValidToUse(unsigned ctime, unsigned cworker) const {
  if (cworker >= windex)
    return ctime >= time + 1;
  else
    return ctime >= time + 2;
}

bool BoosterTime::ValidToUse(const BoosterTime& tnow) const {
  return ValidToUse(tnow.time, tnow.windex);
}
