#pragma once

#include "base/action.h"
#include "base/sleep.h"
#include "base/world_task_split.h"
#include "solvers/solver.h"
#include <algorithm>
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
  void AssignTasks(unsigned new_tasks) {
    ALWAYS_ASSERT(world.WCount() <= worker_solvers.size());
    unsigned task_id = 0;
    for (unsigned i = 0; i < world.WCount(); ++i) {
      auto& w = world.GetWorker(i);
      if (w.task_assigned) {
        worker_solvers[i].ResetTask(task_id++ % new_tasks);
      }
    }
  }

  void SplitTasks(unsigned new_tasks) {
    thread_local std::vector<UnsignedSet> tasks;
    auto v = world.UList();
    if (v.size() < new_tasks) return;
    new_tasks = std::min(new_tasks, unsigned(v.size()));
    std::sort(v.begin(), v.end());
    tasks.resize(new_tasks);
    for (unsigned i = 0; i < new_tasks; ++i) {
      tasks[i].Resize(world.Size());
      tasks[i].Clear();
      for (uint64_t j = (i * v.size()) / new_tasks;
           j < ((i + 1) * v.size()) / new_tasks; ++j) {
        tasks[i].Insert(v[j]);
      }
    }
    world.SetNewTasks(tasks);
    AssignTasks(new_tasks);
  }

  void Init(const std::string& task, const std::string& bonuses) {
    world.Init(task);
    world.InitBonuses(bonuses);
    unsigned max_workers =
        world.boosters.clones.Size() + world.map.Count(Item::CLONE) + 1;
    clone_solver.Init(world);
    worker_solvers.resize(max_workers, worker_example);
    clone_stage_done = false;
  }

  ActionsList NextMove() {
    ALWAYS_ASSERT(world.WCount() <= worker_solvers.size());
    unsigned assigned_workers = 0;
    bool split_required = false;
    ActionsList al(world.WCount(), ActionType::END);
    if (!clone_stage_done) {
      bool still_working = false;
      al = clone_solver.NextMove();
      ALWAYS_ASSERT(al.size() == world.WCount());
      for (unsigned i = 0; i < al.size(); ++i) {
        auto& w = world.GetWorker(i);
        if (w.task_assigned) {
          assigned_workers += 1;
          if (world.GetTask(worker_solvers[i].tindex).Empty())
            split_required = true;
          ALWAYS_ASSERT(al[i].type == ActionType::END);
        } else {
          if (al[i].type == ActionType::END) {
            worker_solvers[i].Init(i, 0, world);
            assigned_workers += 1;
          } else {
            still_working = true;
          }
        }
      }
      if (!still_working) {
        clone_stage_done = true;
      }
    } else {
      assigned_workers = world.WCount();
      for (unsigned i = 0; i < world.WCount(); ++i) {
        if (world.GetTask(worker_solvers[i].tindex).Empty())
          split_required = true;
      }
    }
    if (world.UList().size() <= 2 * assigned_workers) {
      if (world.TotalTasks() != 1) SplitTasks(1);
    } else if ((assigned_workers > world.TotalTasks()) || split_required) {
      SplitTasks(assigned_workers);
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
