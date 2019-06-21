#include "solvers/solve.h"

#include "base/action_encode.h"
#include "solvers/base_greedy.h"
#include "solvers/test.h"
#include "common/timer.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

namespace solvers {
void Solve(const std::string& input_file, const std::string& output_file) {
  std::ifstream input(input_file);
  std::string task;
  assert(std::getline(input, task));
  std::string task_index = input_file.substr(input_file.size() - 8, 3);
  std::cout << "Solving task " << task_index << " ...";
  Timer t;
  BaseGreedy s;
  auto actions = s.Solve(task);
  std::cout << " Done. Time = " << t.GetMilliseconds() << std::endl;
  if (!Test(task, actions)) {
    std::cerr << "Solution for problem " << task_index << " is incorrect."
              << std::endl;
  }
  std::ofstream output(output_file);
  output << actions;
}
}  // namespace solvers
