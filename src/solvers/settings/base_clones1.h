#pragma once

#include <cstddef>

namespace solvers {
class BaseClones1Settings {
 public:
  size_t manip_index{};
  size_t strategy{};
  size_t ext_dist{};
  bool use_shifts{};
  bool even_mops{};
  bool use_teleports{};
  bool all_rotate_and_shift{};
};
}  // namespace solvers
