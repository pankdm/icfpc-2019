#pragma once

#include "action_type.h"
#include <cassert>
#include <vector>

class Direction {
 public:
  unsigned direction;

 public:
  Direction(unsigned _d) : direction(_d & 3) {}
  Direction(ActionType t) { Set(t); }

  int DX() const {
    static const std::vector<int> dx{-1, 0, 1, 0};
    return dx[direction];
  }

  int DY() const {
    static const std::vector<int> dy{0, 1, 0, -1};
    return dy[direction];
  }

  ActionType Get() const {
    static const std::vector<ActionType> v{
        ActionType::MOVE_RIGHT, ActionType::MOVE_UP, ActionType::MOVE_LEFT,
        ActionType::MOVE_DOWN};
    return v[direction];
  }

  void Set(ActionType t) {
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
        assert(false);
    }
  }
};
