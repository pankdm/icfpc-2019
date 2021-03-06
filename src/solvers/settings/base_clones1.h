#pragma once

#include <cstddef>

namespace solvers {
class BaseClones1Settings {
 public:
  size_t manip_index{};
  size_t strategy{};
  size_t ext_dist{};
  bool use_shifts{};
  bool use_teleports{};
  bool all_rotate_and_shift{};
  bool sorted_points{};
  bool use_fast_wheels{};
  bool is_manip(int index) { return manip_index == -1 || manip_index == index; }
};
}  // namespace solvers
