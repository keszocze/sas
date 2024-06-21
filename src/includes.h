#pragma once

#include <vector>

#include "../../base/main/main.h"
#include "../../base/main/mainInt.h"
#include "../../bdd/cudd/cudd.h"
#include "../../bdd/cudd/cuddInt.h"

namespace symmetrize {

using TruthValue = bool;
using ValueCount = double;
using ValueCountsHW = std::vector<ValueCount>;
using ValueVector = std::vector<TruthValue>;

struct SymmetricFunction {
  size_t n, m;
  std::vector<ValueVector> components;
  std::vector<ValueCount> hamming_distances;
};

} // namespace symmetrize
