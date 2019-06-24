#include "base/action_decode.h"

#include "base/action.h"
#include "base/action_type.h"
#include "base/point.h"
#include "utils/split.h"
#include "common/always_assert.h"
#include <string>
#include <vector>

ActionsList ParseAL(const std::string& s) {
  ActionsList al;
  for (unsigned i = 0; i < s.size(); ++i) {
    switch (s[i]) {
      case 'W':
        al.emplace_back(ActionType::MOVE_UP);
        break;
      case 'S':
        al.emplace_back(ActionType::MOVE_DOWN);
        break;
      case 'A':
        al.emplace_back(ActionType::MOVE_LEFT);
        break;
      case 'D':
        al.emplace_back(ActionType::MOVE_RIGHT);
        break;
      case 'Z':
        al.emplace_back(ActionType::DO_NOTHING);
        break;
      case 'E':
        al.emplace_back(ActionType::ROTATE_CLOCKWISE);
        break;
      case 'Q':
        al.emplace_back(ActionType::ROTATE_COUNTERCLOCKWISE);
        break;
      case 'B': {
        al.emplace_back(ActionType::ATTACH_MANIPULATOR);
        size_t npos = s.find(')', i++);
        ALWAYS_ASSERT(npos != std::string::npos);
        ALWAYS_ASSERT(s[i] == '(');
        Point p(s.substr(i, npos - i + 1));
        i = npos;
        al.back().x = p.x;
        al.back().y = p.y;
      } break;
      case 'F':
        al.emplace_back(ActionType::ATTACH_FAST_WHEELS);
        break;
      case 'L':
        al.emplace_back(ActionType::USING_DRILL);
        break;
      case 'R':
        al.emplace_back(ActionType::SET_BEACON);
        break;
      case 'T': {
        al.emplace_back(ActionType::SHIFT);
        size_t npos = s.find(')', i++);
        ALWAYS_ASSERT(npos != std::string::npos);
        ALWAYS_ASSERT(s[i] == '(');
        Point p(s.substr(i, npos - i + 1));
        i = npos;
        al.back().x = p.x;
        al.back().y = p.y;
      } break;
      case 'C':
        al.emplace_back(ActionType::CLONE);
        break;
      default:
        ALWAYS_ASSERT(false);
        break;
    }
  }
  return al;
}

ActionsClones ParseAC(const std::string& s) {
  ActionsClones output;
  for (const std::string& ss : Split(s, '#')) {
    output.emplace_back(ParseAL(ss));
  }
  return output;
}
