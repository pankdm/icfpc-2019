#pragma once

#include "base/action.h"
#include <ostream>

std::ostream& operator<<(std::ostream& os, const Action& a);
std::ostream& operator<<(std::ostream& os, const ActionsList& al);
