#pragma once

#include "base/action.h"
#include "base/sleep.h"
#include "base/world_task_split.h"
#include "solvers/solver.h"
#include <vector>

namespace solvers {
template <class TCloneSolver, class TWorkerSolver>
class TaskSplitter : public Solver {
 protected:
  WorldTaskSplit world;
  TCloneSolver clone_solver;
  TWorkerSolver worker_example;
  std::vector<TWorkerSolver> worker_solvers;
  bool clone_stage_done;

 protected:
  void Init(const std::string& task, const std::string& bonuses) {
    world.Init(task);
    world.InitBonuses(bonuses);
    // world.SetNewTasks(std::vector<UnsignedSet>(1, world.UnwrappedSet()));
    unsigned max_workers =
        world.boosters.clones.Size() + world.map.Count(Item::CLONE) + 1;
    clone_solver.Init(world);
    worker_solvers.resize(max_workers, worker_example);
    clone_stage_done = false;
  }

  ActionsList NextMove() {
    ALWAYS_ASSERT(world.WCount() <= worker_solvers.size());
    ActionsList al(world.WCount(), ActionType::END);
    if (!clone_stage_done) {
      bool still_working = false;
      al = clone_solver.NextMove();
      ALWAYS_ASSERT(al.size() == world.WCount());
      for (unsigned i = 0; i < al.size(); ++i) {
        auto& w = world.GetWorker(i);
        if (w.task_assigned) {
          ALWAYS_ASSERT(al[i].type == ActionType::END);
        } else {
          if (al[i].type == ActionType::END) {
            worker_solvers[i].Init(i, 0, world);
          } else {
            still_working = true;
          }
        }
      }
      if (!still_working) {
        // Do something!
        clone_stage_done = true;
      }
    }
    for (unsigned i = 0; i < al.size(); ++i) {
      if (al[i].type == ActionType::END) {
        al[i] = worker_solvers[i].NextMove();
      }
    }
    for (auto& a : al) {
      ALWAYS_ASSERT(a.type != ActionType::END)
    }
    return al;
  }

 public:
  TaskSplitter(const TCloneSolver& _clone_solver,
               const TWorkerSolver& _worker_solver) {
    clone_solver = _clone_solver;
    worker_example = _worker_solver;
  }

  ActionsClones Solve(const std::string& task, const std::string& bonuses) {
    Init(task, bonuses);
    ActionsClones actions;
    for (; !world.Solved();) {
      auto al = NextMove();
      if (Sleep(al)) break;
      world.ApplyC(al);
      if (actions.size() < al.size()) {
        actions.resize(al.size());
      }
      for (unsigned i = 0; i < al.size(); ++i) actions[i].emplace_back(al[i]);
    }
    if (actions.size() < world.WCount()) {
      actions.resize(world.WCount());
    }
    return actions;
  }
};
}  // namespace solvers
