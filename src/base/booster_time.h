#pragma once

class BoosterTime {
 public:
  unsigned time = 0;
  unsigned windex = 0;

  bool ValidToUse(unsigned ctime, unsigned cworker) const;
  bool ValidToUse(const BoosterTime& tnow) const;
};
