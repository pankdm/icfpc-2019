#include "solvers/solve.h"

#include "base/action_encode.h"
#include "solvers/base_greedy2.h"
#include "solvers/base_greedy3.h"
#include "solvers/optimization_teleport.h"
#include "solvers/test.h"
#include "common/always_assert.h"
#include "common/timer.h"
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
  BaseGreedy3 s;
  auto actions = s.Solve(task);

  Timer t2;
  TeleportOptimization opt;
  auto new_actions = opt.apply(task, actions);
  if (new_actions.size() < actions.size()) {
    std::cout << "Task " << task_index << " was optimized from "
              << actions.size() << " to " << new_actions.size() << ", opt time = "
              << t2.GetMilliseconds() << std::endl;
    std::swap(actions, new_actions);
  }

  unsigned score = Test(task, actions);
  std::cout << "Task " << task_index << " Done. Time = " << t.GetMilliseconds()
            << "\tScore = " << score << std::endl;
  if (score) {
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
