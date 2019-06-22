#include <future>
#include <iostream>
#include <string>

#include "common/always_assert.h"
#include "common/pool.h"
#include "common/timer.h"

#include "solvers/solve.h"


// int main() {
//   bool b = solvers::Solve("../problems/part-2-teleports-examples/example-02.desc", "2.sol");
//   if (b) {
//     std::cerr << "Success!" << std::endl;
//   } else {
//     std::cerr << "Fail!" << std::endl;
//   }
//   return 0;
// }


int main() {
  // ALWAYS_ASSERTF(2 != 3, "%d %s\n", 3, "str");

  Timer t;
  ThreadPool p(30);
  std::atomic<bool> all_ok(true);
  std::vector<std::future<void>> futures;
  for (unsigned i = 1; i <= 300; ++i) {
    auto t = std::make_shared<std::packaged_task<void()>>([&, i]() {
      std::string si = std::to_string(i + 1000).substr(1);
      bool b = solvers::Solve("../problems/all/prob-" + si + ".desc",
                              "../solutions_cpp/prob-" + si + ".sol");
      all_ok = all_ok && b;
    });
    futures.emplace_back(p.enqueueTask(std::move(t)));
  }
  for (auto& f : futures) {
    f.get();
  }
  std::cout << "Total time = " << t.GetMilliseconds() << std::endl;
  if (!all_ok) {
    std::cerr << "Some solutions failed" << std::endl;
  }
  return 0;
}
