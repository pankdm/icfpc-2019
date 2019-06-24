#include "solvers/auto.h"

#include "common/command_line.h"
#include "common/pool.h"

#include "solvers/base_clones.h"
#include "solvers/base_clones1.h"
#include "solvers/base_greedy2.h"
#include "solvers/base_greedy3.h"
#include "solvers/clone/base.h"
#include "solvers/clones_greedy.h"
#include "solvers/file.h"
#include "solvers/merger.h"
#include "solvers/task_splitter.h"
#include "solvers/worker/greedy.h"
#include "solvers/worker/local.h"

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

    if (!cmd.args["current_best"].empty()) {
      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            File fsolver;
            return Result("fsr",
                          fsolver.Solve(task, "", cmd.args["current_best"]));
          })));
    }

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

    for (unsigned i = 0; i < 2; ++i) {
      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&, i]() {
            ClonesGreedy cg0;
            return Result("cg0", cg0.Solve(task, i, bonuses));
          })));
    }

    if (!cmd.int_args["nobc"]) {
      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc1;
            BaseClones1Settings sett{0, 0, 100, true, false};
            return Result("bc1", bc1.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones bc0;
            return Result("bc0", bc0.Solve(task, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc2;
            BaseClones1Settings sett{1, 0, 100, true, false};
            return Result("bc2", bc2.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc3;
            BaseClones1Settings sett{0, 1, 100, true, false};
            return Result("bc3", bc3.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc1;
            BaseClones1Settings sett{0, 0, 10, true, false};
            return Result("bc4", bc1.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc2;
            BaseClones1Settings sett{1, 0, 10, true, false};
            return Result("bc5", bc2.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc3;
            BaseClones1Settings sett{0, 1, 10, true, false};
            return Result("bc6", bc3.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc3;
            BaseClones1Settings sett{0, 0, 10, true, false, true};
            return Result("bc_t", bc3.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc3;
            BaseClones1Settings sett{0, 0, 100, true, false, true};
            return Result("bc_t", bc3.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc2;
            BaseClones1Settings sett{1, 0, 100, true, false, true, true};
            return Result("bc2_sober", bc2.Solve(task, sett, bonuses));
          })));

      futures.emplace_back(tp->enqueueTask<Result>(
          std::make_shared<std::packaged_task<Result()>>([&]() {
            BaseClones1 bc2;
            BaseClones1Settings sett{1, 0, 10, true, false, false, true};
            return Result("bc5_sober", bc2.Solve(task, sett, bonuses));
          })));
    }
    /*
     */

    for (auto& f : futures) {
      auto res = f.get();
      m.AddSolution(res.second, res.first);
    }

  } else {
    // Never comment file solver!
    File fsolver;
    m.AddSolution(fsolver.Solve(task, task_name), "fsr");
    // End "never comment" section

    /*
    BaseClones1Settings sett{0, 0, 0, 0, true, 1};
    BaseClones1 bc;
    m.AddSolution(bc.Solve(task, sett, bonuses), std::string("bct"));
    */

    for (unsigned i = 0; i < 32; i++) {
      for (int j = 0; j < 40; j += 10) {
        BaseClones1Settings sett{i & 1, i & 2, j,    i & 4,
                                 true,  i & 8, true, i & 16};
        BaseClones1 bc;
        m.AddSolution(
            bc.Solve(task, sett, bonuses),
            std::string("bc_") + std::to_string(i) + "_" + std::to_string(j));
      }
    }
    /*
     */

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
    // m.AddSolution(bc0.Solve(task, bonuses), "bc0");
    // for (unsigned i = 0; i < 2; ++i) {
    //   ClonesGreedy cg0;
    //   m.AddSolution(cg0.Solve(task, i, bonuses), "cg0");
    // }

    // clone::Base clone_solver;
    // worker::Greedy worker_greedy;
    // worker::Local worker_local;
    // TaskSplitter<clone::Base, worker::Greedy> tsp_greedy(clone_solver,
    //                                                      worker_greedy);
    // TaskSplitter<clone::Base, worker::Local> tsp_local(clone_solver,
    //                                                    worker_local);
    // m.AddSolution(tsp_greedy.Solve(task, bonuses), "tsp_g");
    // m.AddSolution(tsp_local.Solve(task, bonuses), "tsp_l");
  }
  return m.Solution();
}
}  // namespace solvers
