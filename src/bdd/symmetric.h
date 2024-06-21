#pragma once

#include "bdd.h"

namespace symmetrize {
namespace bdd {

// Creates the BDDs for the given symmetric function
// see: A. Bernasconi, V. Ciriani and T. Villa, "Exploiting Symmetrization
// and D-Reducibility for Approximate Logic Synthesis,"
// in IEEE Transactions on Computers, vol. 71, no. 1, pp. 121-133,
// 1 Jan. 2022, doi: 10.1109/TC.2020.3043476.
BDDs Create(DdManager *mgr, const SymmetricFunction &f);

} // namespace bdd
} // namespace symmetrize
