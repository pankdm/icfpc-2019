#include "solvers/optimization_teleport.h"

#include "common/always_assert.h"

#include <algorithm>
#include <iostream>

namespace solvers {

struct TeleportTask {
  unsigned set_beacon_time = 0;
  // location of teleport to jump to
  int x;
  int y;
  unsigned teleport_time = 0;
  unsigned continue_time = 0;

  int benefit = 0;

  bool operator<(const TeleportTask& other) const {
    return benefit < other.benefit;
  }
};

ActionsList TeleportOptimization::apply(const std::string& task,
                                        const ActionsList& actions) {
  World world;
  world.map.save_wraps = true;
  world.Init(task);

  std::vector<TeleportTask> tasks;

  // keep track of first visited time when carrying teleport
  std::vector<unsigned> visited_time(world.map.Size(), 0);
  unsigned unchanged_duration = 0;

  for (const auto& action : actions) {
    int prev_size = world.map.wraps_history.size();
    world.Apply(action);

    if (world.map.wraps_history.size() == prev_size) {
      unchanged_duration += 1;
    } else {
      unchanged_duration = 0;
    }
    int x = world.workers[0].x;
    int y = world.workers[0].y;

    // std::cerr << "time = " << world.time
    //           << " unchanged = " << unchanged_duration << " x,y = " << x <<
    //           ","
    //           << y << std::endl;

    auto index = world.map.Index(x, y);
    if (unchanged_duration > 2 && visited_time[index] > 0) {
      TeleportTask task{
          .set_beacon_time = visited_time[index],
          .x = x,
          .y = y,
          .teleport_time = world.time - unchanged_duration,
          .continue_time = world.time,
          .benefit = unchanged_duration - 2,
      };
      tasks.emplace_back(task);

      // std::cerr << "adding task, set: " << task.set_beacon_time
      //           << ", teleport_time: " << task.teleport_time
      //           << " benefit: " << task.benefit
      //           << " continue: " << task.continue_time << std::endl;
    }

    if (world.boosters.teleporters.Available({world.time, 0}) &&
        visited_time[index] == 0) {
      visited_time[index] = world.time;
    }
  }

  if (tasks.empty()) {
    // no optimizations found
    return actions;
  }
  // sort in order from best to worst
  std::sort(tasks.begin(), tasks.end());
  std::reverse(tasks.begin(), tasks.end());
  const auto& best = tasks[0];

  // std::cerr << "using best task, set: " << best.set_beacon_time
  //           << ", teleport_time: " << best.teleport_time
  //           << " benefit: " << best.benefit
  //           << " continue: " << best.continue_time << std::endl;

  ActionsList result;
  result.reserve(actions.size());

  int current_time = 0;
  for (const auto& action : actions) {
    ++current_time;

    if (best.teleport_time < current_time &&
        current_time <= best.continue_time) {
      continue;
    }
    result.emplace_back(action);
    if (current_time == best.set_beacon_time) {
      result.emplace_back(Action(ActionType::SET_BEACON));
    }
    if (current_time == best.teleport_time) {
      Action shift_action(ActionType::SHIFT);
      shift_action.x = best.x;
      shift_action.y = best.y;
      result.emplace_back(shift_action);
    }
  }
  ALWAYS_ASSERTF(result.size() + best.benefit == actions.size(),
                 "%d + %d != %d\n", (int)result.size(), (int)best.benefit,
                 (int)actions.size());
  return result;
}

}  // namespace solvers
