#include "base/action.h"

#include "base/action_type.h"

Action::Action() : type(ActionType::END), x(0), y(0) {}
Action::Action(ActionType _type) : Action() { type = _type; }
Action::Action(ActionType _type, int _x, int _y) : Action() { 
    type = _type; 
    x = _x;
    y = _y;
}
