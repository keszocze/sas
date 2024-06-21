#pragma once

#include "../utils/maths.h"
#include "bdd.h"

namespace symmetrize {
namespace bdd {

// Calculates C_H(f) for the function f with BDDs bdds, n variables and binomial
// coefficients up to n over n.
std::vector<ValueCountsHW>
C_H(const BDDs &bdds, int n, const BinomialCoefficients<ValueCount> &binomial);

} // namespace bdd
} // namespace symmetrize
