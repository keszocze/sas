#pragma once

#include "../includes.h"
#include <stdexcept>

namespace symmetrize {
namespace commands {

bool ToSize(const char *txt, size_t &res);
bool ToDouble(const char *txt, double &res);

} // namespace commands
} // namespace symmetrize
