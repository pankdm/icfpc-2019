#pragma once

#include "base/action.h"
#include "base/action_type.h"

bool Sleep(ActionType type);
bool Sleep(const Action& action);
bool Sleep(const ActionsList& actions);
