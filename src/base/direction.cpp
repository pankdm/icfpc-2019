#include "base/direction.h"

#include "base/map.h"
#include "common/always_assert.h"
#include <cmath>
#include <vector>

Direction::Direction(unsigned _d) : direction(_d & 3) {}
Direction::Direction(ActionType t) { Set(t); }

int Direction::DX() const {
  static const std::vector<int> dx{1, 0, -1, 0};
  return dx[direction];
}

int Direction::DY() const {
  static const std::vector<int> dy{0, 1, 0, -1};
  return dy[direction];
}

ActionType Direction::Get() const {
  static const std::vector<ActionType> v{
      ActionType::MOVE_RIGHT, ActionType::MOVE_UP, ActionType::MOVE_LEFT,
      ActionType::MOVE_DOWN};
  return v[direction];
}

void Direction::Set(ActionType t) {
  switch (t) {
    case ActionType::MOVE_RIGHT:
      direction = 0;
      break;
    case ActionType::MOVE_UP:
      direction = 1;
      break;
    case ActionType::MOVE_LEFT:
      direction = 2;
      break;
    case ActionType::MOVE_DOWN:
      direction = 3;
      break;
    default:
      ALWAYS_ASSERT(false);
  }
}

Direction GetDirection(const Map& map, int from, int to) {
  int d = from - to;
  int dabs = std::abs(d);
  ALWAYS_ASSERT((dabs == 1) || (dabs == map.ysize));
  if (to == from + 1)
    return 1;
  else if (to == from - 1)
    return 3;
  else if (to > from)
    return 0;
  else
    return 2;
}
