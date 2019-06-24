#include "base/action_encode.h"

#include "base/action_type.h"
#include "common/always_assert.h"

std::ostream& operator<<(std::ostream& os, const Action& a) {
  switch (a.type) {
    case ActionType::MOVE_UP:
      return os << "W";
    case ActionType::MOVE_DOWN:
      return os << "S";
    case ActionType::MOVE_LEFT:
      return os << "A";
    case ActionType::MOVE_RIGHT:
      return os << "D";
    case ActionType::DO_NOTHING:
      return os << "Z";
    case ActionType::ROTATE_CLOCKWISE:
      return os << "E";
    case ActionType::ROTATE_COUNTERCLOCKWISE:
      return os << "Q";
    case ActionType::ATTACH_MANIPULATOR:
      return os << "B(" << a.x << "," << a.y << ")";
    case ActionType::ATTACH_FAST_WHEELS:
      return os << "F";
    case ActionType::USING_DRILL:
      return os << "L";
    case ActionType::SET_BEACON:
      return os << "R";
    case ActionType::SHIFT:
      return os << "T(" << a.x << "," << a.y << ")";
    case ActionType::CLONE:
      return os << "C";
  }
  ALWAYS_ASSERT(false);
  return os;
}

std::ostream& operator<<(std::ostream& os, const ActionsList& al) {
  for (auto a : al) os << a;
  return os;
}

std::ostream& operator<<(std::ostream& os, const ActionsClones& al) {
  ALWAYS_ASSERT(al.size() > 0);
  os << al[0];
  for (unsigned i = 1; i < al.size(); ++i) os << "#" << al[i];
  return os;
}
