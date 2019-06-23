#include "solvers/file.h"

#include "base/action_decode.h"
#include "common/always_assert.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace solvers {
ActionsClones File::Solve(const std::string& task,
                          const std::string& task_name) {
  auto filename = "../solutions_cpp/prob-" + task_name + ".sol";
  std::ifstream f(filename);
  std::string s;
  if (!f.is_open()) return {};
  if (!std::getline(f, s)) return {};
  return ParseAC(s);
}
}  // namespace solvers
