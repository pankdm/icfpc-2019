#include "base/sleep.h"

bool Sleep(ActionType type) { return type == ActionType::DO_NOTHING; }
bool Sleep(const Action& action) { return Sleep(action.type); }

bool Sleep(const ActionsList& actions) {
  for (auto& action : actions) {
    if (!Sleep(action)) return false;
  }
  return true;
}
