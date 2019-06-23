#include "solvers/merger.h"

#include "solvers/test.h"
#include <iostream>

namespace solvers {
Merger::Merger(const std::string& _task, const std::string& _task_name)
    : task(_task), task_name(_task_name) {}

void Merger::AddSolution(const ActionsClones& s,
                         const std::string& solution_name) {
  unsigned score = Test(task, s);
  if (solution_name != "") {
    std::cout << "Task " << task_name << "\tSolution " << solution_name
              << "\tScore = " << score << std::endl;
  }
  if (score > 0 && ((best_score == 0) || (score < best_score))) {
    best_score = score;
    best_solutions = s;
  }
}

void Merger::AddSolution(const ActionsList& s,
                         const std::string& solution_name) {
  ActionsClones sc(1, s);
  AddSolution(sc, solution_name);
}
}  // namespace solvers
