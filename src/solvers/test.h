#pragma once

#include "base/action.h"
#include <string>

namespace solvers {
unsigned Test(const std::string& task, const ActionsList& actions);
unsigned Test(const std::string& task, const ActionsClones& actions);
}  // namespace solvers
