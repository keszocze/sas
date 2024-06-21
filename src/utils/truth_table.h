#pragma once

#include <string>
#include <vector>

#include "../includes.h"

namespace symmetrize {

// tt[i] is value of f(i_{bin})
using TruthTable = std::vector<TruthValue>;

namespace tt {

std::string ToString(const TruthTable &table, bool fill_dc = false);

TruthTable FillMinBeads(const TruthTable &table);

} // namespace tt

} // namespace symmetrize
