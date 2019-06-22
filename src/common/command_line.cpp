#include "command_line.h"

#include <cstdlib>

#include <thread>

void CommandLine::Parse(int argc, char* const argv[]) {
  int_args["threads"] =
      std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);

  int index = 1;
  while (index + 1 < argc) {
    std::string arg = argv[index];
    if (arg[0] == '-') {
      arg = arg.substr(1);
    }
    args[arg] = argv[index + 1];
    int_args[arg] = atoi(argv[index + 1]);
    index += 2;
  }
}

CommandLine cmd;
