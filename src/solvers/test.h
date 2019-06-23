#pragma once

#include "base/action.h"
#include <string>

namespace solvers {
unsigned Test(const std::string& task, const ActionsList& actions,
              const std::string& bonuses);
unsigned Test(const std::string& task, const ActionsClones& actions,
              const std::string& bonuses);
}  // namespace solvers
