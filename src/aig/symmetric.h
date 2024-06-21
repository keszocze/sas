#pragma once

#include "../includes.h"

#include "network.h"

namespace symmetrize {
namespace aig {

// Adds POs realizing f with respect to ntk's PIs assuming ntk is an AIG
Signals AddSymmetricPOs(Abc_Ntk_t *ntk, const SymmetricFunction &f);

// Adds POs realizing f with respect inputs assuming ntk is an AIG
Signals AddSymmetricPOs(Abc_Ntk_t *ntk, const SymmetricFunction &f,
                        const Signals &inputs);

// Creates a new AIG realizing the given symmetric function
Abc_Ntk_t *CreateSymmetricNetwork(const SymmetricFunction &f);

} // namespace aig
} // namespace symmetrize
