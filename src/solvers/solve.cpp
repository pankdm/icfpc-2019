#include "solvers/solve.h"

#include "base/action_encode.h"
#include "solvers/base_greedy2.h"
#include "solvers/test.h"
#include "common/timer.h"
#include "common/always_assert.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

namespace solvers {
bool Solve(const std::string& input_file, const std::string& output_file) {
  std::ifstream input(input_file);
  std::string task;
  ALWAYS_ASSERT(std::getline(input, task));
  std::string task_index = input_file.substr(input_file.size() - 8, 3);
  Timer t;
  BaseGreedy2 s;
  auto actions = s.Solve(task);
  std::cout << "Task " << task_index << " Done. Time = " << t.GetMilliseconds()
            << "\tScore = " << actions.size() << std::endl;
  if (Test(task, actions)) {
    std::ofstream output(output_file);
    output << actions;
    return true;
  } else {
    std::cerr << "Solution for problem " << task_index << " is incorrect."
              << std::endl;
    return false;
  }
}
}  // namespace solvers
