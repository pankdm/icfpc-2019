#pragma once

#include "base/action.h"
#include <string>

namespace solvers {
class Merger {
 protected:
  std::string task;
  std::string task_name;
  std::string bonuses;
  unsigned best_score = 0;
  ActionsClones best_solutions;

 public:
  Merger(const std::string& _task, const std::string& _task_name,
         const std::string& _bonuses);
  void AddSolution(const ActionsClones& s,
                   const std::string& solution_name = "");
  void AddSolution(const ActionsList& s, const std::string& solution_name = "");

  unsigned Score() const { return best_score; }
  ActionsClones Solution() const { return best_solutions; }
};
}  // namespace solvers
