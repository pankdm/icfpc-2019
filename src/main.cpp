#include <future>
#include <iostream>
#include <string>

#include "common/always_assert.h"
#include "common/command_line.h"
#include "common/pool.h"
#include "common/timer.h"

#include "solvers/solve.h"

// int main() {
//   bool b =
//   solvers::Solve("../problems/part-2-teleports-examples/example-02.desc",
//   "2.sol"); if (b) {
//     std::cerr << "Success!" << std::endl;
//   } else {
//     std::cerr << "Fail!" << std::endl;
//   }
//   return 0;
// }

int main(int argc, char* argv[]) {
  cmd.Parse(argc, argv);

  if (cmd.int_args["solve"]) {
    auto in = cmd.args["in"];
    auto out = cmd.args["out"];
    /*
    Adds support for initial bonus boosters for the experimental runs
        Example: src$ build/cpp_solver -solve 1 -in
    ../lambda-client/data/task13.desc -out temp.log -bonus
    ../lambda-client/bonuses.buy
     */
    auto bonus = cmd.args["bonus"];
    solvers::Solve(in, out, bonus, "ext");
    return 0;
  }

  Timer t;
  ThreadPool p(cmd.int_args["threads"]);
  std::atomic<bool> all_ok(true);
  std::vector<std::future<int>> futures;
  for (unsigned i = 221; i <= 300; ++i) {
    auto t = std::make_shared<std::packaged_task<int()>>([&, i]() {
      std::string si = std::to_string(i + 1000).substr(1);
      unsigned num_steps =
          solvers::Solve("../problems/all/prob-" + si + ".desc",
                         "../solutions_cpp/prob-" + si + ".sol",
                         "",  // empty bonus file path for now
                         si);
      all_ok = all_ok && num_steps;
      return num_steps;
    });
    futures.emplace_back(p.enqueueTask(std::move(t)));
  }
  unsigned total_steps = 0;
  for (auto& f : futures) {
    total_steps += f.get();
  }
  std::cout << "Total time = " << t.GetMilliseconds() << std::endl;
  std::cout << "Total steps = " << total_steps << std::endl;
  if (!all_ok) {
    std::cerr << "Some solutions failed" << std::endl;
  }
  return 0;
}
