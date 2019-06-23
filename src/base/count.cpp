#include "base/count.h"

#include <algorithm>

unsigned Count(const std::string& task, Item item) {
  char item_code = ' ';
  switch (item) {
    case Item::EXTENSION:
      item_code = 'B';
      break;
    case Item::FAST_WHEELS:
      item_code = 'F';
      break;
    case Item::DRILL:
      item_code = 'L';
      break;
    case Item::TELEPORT:
      item_code = 'R';
      break;
    case Item::CODEX:
      item_code = 'X';
      break;
    case Item::CLONE:
      item_code = 'C';
      break;
  }
  return std::count(task.begin(), task.end(), item_code);
}
