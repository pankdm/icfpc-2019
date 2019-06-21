#include "solutions/solve.h"
#include <string>

int main() {
  for (unsigned i = 1; i <= 220; ++i) {
    std::string si = std::to_string(i + 1000).substr(1);
    solutions::Solve("../problems/all/prob-" + si + ".desc",
                     "../solutions_cpp/prob-" + si + ".sol");
  }
  return 0;
}
