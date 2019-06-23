#include "solvers/auto.h"

#include "solvers/base_clones.h"
#include "solvers/base_greedy2.h"
#include "solvers/base_greedy3.h"
#include "solvers/file.h"
#include "solvers/merger.h"

namespace solvers {
ActionsClones Auto::Solve(const std::string& task,
                          const std::string& task_name) {
  Merger m(task, task_name);
  if (task_name == "ext") {
    BaseGreedy2 bg2;
    m.AddSolution(bg2.Solve(task), "bg2");
    BaseGreedy3 bg3(BaseGreedy3Settings{true, true, 0});
    m.AddSolution(bg3.Solve(task), "bg3");
    BaseClones bc0;
    m.AddSolution(bc0.Solve(task), "bc0");
  } else {
    File fsolver;
    m.AddSolution(fsolver.Solve(task, task_name), "fsr");
    // BaseGreedy2 bg2;
    // m.AddSolution(bg2.Solve(task), "bg2");
    // for (unsigned i = 0; i < 4; ++i) {
    //   BaseGreedy3 bg3(BaseGreedy3Settings{i & 1, i & 2, 0});
    //   m.AddSolution(bg3.Solve(task), "bg3");
    // }
    // BaseClones bc0;
    // m.AddSolution(bc0.Solve(task), "bc0");
  }
  return m.Solution();
}
}  // namespace solvers
