#pragma once

namespace solvers {
class BaseGreedy3Settings {
 public:
  bool use_sword;
  bool use_shifts;
  int chase_mops_cnt;
  int dist_weight;  // 0 = old algo, 1 = chase argmin(dist+size),
                    // 2 = chase rgmin(2*dist+size) (prefer closest) etc.
};
}  // namespace solvers
