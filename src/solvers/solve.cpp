#include "solvers/solve.h"

#include "base/action_encode.h"
#include "solvers/base_greedy.h"
#include "solvers/test.h"
#include <cassert>
#include <fstream>
#include <string>

namespace solvers {
void Solve(const std::string& input_file, const std::string& output_file) {
  std::ifstream input(input_file);
  std::string task;
  assert(std::getline(input, task));
  BaseGreedy s;
  auto actions = s.Solve(task);
  assert(Test(task, actions));
  std::ofstream output(output_file);
  output << actions;
}
}  // namespace solvers
