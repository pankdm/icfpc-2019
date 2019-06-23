#include "solvers/solve.h"

#include "base/action_encode.h"
#include "solvers/auto.h"
#include "solvers/test.h"
#include "common/always_assert.h"
#include "common/timer.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

namespace solvers {
unsigned Solve(const std::string& input_file, const std::string& output_file,
               const std::string& bonus_file, const std::string& task_name) {
  std::ifstream input(input_file);
  std::string task;
  ALWAYS_ASSERT(std::getline(input, task));
  Timer t;
  Auto s;
  auto actions = s.Solve(task, task_name);
  unsigned score = Test(task, actions);
  std::cout << "Task " << task_name << " Done. Time = " << t.GetMilliseconds()
            << "\tScore = " << score << std::endl;
  if (score) {
    std::ofstream output(output_file);
    output << actions;
  } else {
    std::ofstream output(output_file);
    output << actions;
    std::cerr << "Solution for problem " << task_name << " is incorrect."
              << std::endl;
  }
  return score;
}
}  // namespace solvers
