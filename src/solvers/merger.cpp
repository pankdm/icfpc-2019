#include "solvers/merger.h"

#include <sys/stat.h>

#include <iostream>
#include <fstream>

#include "solvers/test.h"
#include "base/action_encode.h"

namespace solvers {
Merger::Merger(const std::string& _task, const std::string& _task_name,
               const std::string& _bonuses)
    : task(_task), task_name(_task_name), bonuses(_bonuses) {}

void Merger::AddSolution(const ActionsClones& s,
                         const std::string& solution_name) {
  unsigned score = Test(task, s, bonuses);
  if (solution_name != "") {
    std::cout << "Task " << task_name << "\tSolution " << solution_name
              << "\tScore = " << score << std::endl;
  }
  if (score > 0 && ((best_score == 0) || (score < best_score))) {
    best_score = score;
    best_solutions = s;
  }

  char dbuffer[1024];
  sprintf(dbuffer, "solutions");
  mkdir(dbuffer, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  sprintf(dbuffer, "solutions/%s", solution_name.c_str());
  mkdir(dbuffer, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  char buffer[1024];
  sprintf(buffer, "%s/%s.sol", dbuffer, task_name.c_str());
  std::ofstream out(buffer);
  out << s;
}

void Merger::AddSolution(const ActionsList& s,
                         const std::string& solution_name) {
  ActionsClones sc(1, s);
  AddSolution(sc, solution_name);
}
}  // namespace solvers
