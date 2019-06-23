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
    BaseGreedy3 bg3(BaseGreedy3Settings{true, true, 0, 0});
    m.AddSolution(bg3.Solve(task), "bg3");
    BaseGreedy3 bg4(BaseGreedy3Settings{true, true, 100, 2});
    m.AddSolution(bg4.Solve(task), "bg4");
    BaseClones bc0;
    m.AddSolution(bc0.Solve(task), "bc0");
  } else {
    // Never comment file solver!
    File fsolver;
    m.AddSolution(fsolver.Solve(task, task_name), "fsr");
    // End "never comment" section

    // for (unsigned i = 1; i < 3; ++i) {
    //   for (unsigned j = 1; j < 3; ++j) {
    //     for (unsigned k = 0; k < 4; ++k) {
    //       BaseGreedy3 bg3(BaseGreedy3Settings{k & 1, k & 2, i, j});
    //       m.AddSolution(bg3.Solve(task), "bg3");
    //     }
    //   }
    // }
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
