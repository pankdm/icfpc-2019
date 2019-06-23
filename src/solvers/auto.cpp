#include "solvers/auto.h"

#include "common/command_line.h"
#include "common/pool.h"

#include "solvers/base_clones.h"
#include "solvers/base_clones1.h"
#include "solvers/base_greedy2.h"
#include "solvers/base_greedy3.h"
#include "solvers/clones_greedy.h"
#include "solvers/file.h"
#include "solvers/merger.h"

namespace solvers {
ActionsClones Auto::Solve(const std::string& task, const std::string& task_name,
                          const std::string& bonuses) {
  static std::shared_ptr<ThreadPool> tp;
  if (!tp) {
    tp = std::make_shared<ThreadPool>(cmd.int_args["threads"]);
  }

  Merger m(task, task_name, bonuses);
  if (task_name == "ext") {
    using Result = std::pair<std::string, ActionsClones>;

    std::vector<std::future<Result>> futures;

    auto to_action_clones = [](const auto& list) {
      ActionsClones result(1, list);
      return result;
    };

    futures.emplace_back(tp->enqueueTask<Result>(
        std::make_shared<std::packaged_task<Result()>>([&]() {
          BaseGreedy2 bg2;
          return Result("bg2", to_action_clones(bg2.Solve(task)));
        })));

    futures.emplace_back(tp->enqueueTask<Result>(
        std::make_shared<std::packaged_task<Result()>>([&]() {
          BaseGreedy3 bg3(BaseGreedy3Settings{true, true, 0, 0});
          return Result("bg3", to_action_clones(bg3.Solve(task)));
        })));

    futures.emplace_back(tp->enqueueTask<Result>(
        std::make_shared<std::packaged_task<Result()>>([&]() {
          BaseGreedy3 bg4(BaseGreedy3Settings{true, true, 100, 2});
          return Result("bg4", to_action_clones(bg4.Solve(task)));
        })));

    futures.emplace_back(tp->enqueueTask<Result>(
        std::make_shared<std::packaged_task<Result()>>([&]() {
          BaseClones bc0;
          return Result("bc0", bc0.Solve(task, bonuses));
        })));

    futures.emplace_back(tp->enqueueTask<Result>(
        std::make_shared<std::packaged_task<Result()>>([&]() {
          BaseClones1 bc1;
          return Result("bc1", bc1.Solve(task));
        })));

    for (unsigned i = 0; i < 2; ++i) {
      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&, i]() {
            ClonesGreedy cg0;
            return Result("cg0", cg0.Solve(task, i, bonuses));
          })));
    }

    for (auto& f : futures) {
      auto res = f.get();
      m.AddSolution(res.second, res.first);
    }

  } else {
    // Never comment file solver!
    File fsolver;
    m.AddSolution(fsolver.Solve(task, task_name), "fsr");
    // End "never comment" section
    BaseClones1 bc1;
    m.AddSolution(bc1.Solve(task), "bc1");

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
    // for (unsigned i = 0; i < 2; ++i) {
    //   ClonesGreedy cg0;
    //   m.AddSolution(cg0.Solve(task, i), "cg0");
    // }
  }
  return m.Solution();
}
}  // namespace solvers
