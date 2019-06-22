#include "solvers/solve.h"
#include "common/timer.h"
#include <iostream>
#include <string>

int main() {
  bool all_ok = true;
  Timer t;
  for (unsigned i = 1; i <= 300; ++i) {
    std::string si = std::to_string(i + 1000).substr(1);
    bool b = solvers::Solve("../problems/all/prob-" + si + ".desc",
                            "../solutions_cpp/prob-" + si + ".sol");
    all_ok = all_ok && b;
  }
  std::cout << "Total time = " << t.GetMilliseconds() << std::endl;
  if (!all_ok) {
    std::cerr << "Some solutions failed" << std::endl;
  }
  return 0;
}
