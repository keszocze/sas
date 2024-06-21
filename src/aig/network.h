#pragma once

/*
 * Contains utility functions for handling AIGs
 */

#include <limits>
#include <string>
#include <vector>

#include "../bdd/bdd.h"
#include "../includes.h"
#include "../utils/truth_table.h"

namespace symmetrize {
namespace aig {

using Signal = Abc_Obj_t *;
using Signals = std::vector<Signal>;

// Executes Abc_AigCleanup on the AIG manager of ntk and returns
// Abc_NtkCheck(ntk).
bool CleanupAndCheck(Abc_Ntk_t *ntk);

// Creates a new AIG with ni PIs and no POs. Adds dummy names to the inputs
// and outputs if dummy_names is set to true.
Abc_Ntk_t *Create(size_t ni = 0, size_t no = 0, bool dummy_names = true);

// Sets the name of the given network to name.
void SetName(Abc_Ntk_t *ntk, const std::string &name);

// Returns a vector of all POs of the given network
Signals GetPOs(Abc_Ntk_t *ntk);

// Returns a vector of all PIs of the given network
Signals GetPIs(Abc_Ntk_t *ntk);

// For each signal in signals, adds a new PO to ntk and sets its fanin to the
// signal. If dummy_names is set to true, a dummy name is assigned to the PO.
void AddPOs(Abc_Ntk_t *ntk, const Signals &signals, bool dummy_names = true);

// Adds the specified amount of POs to ntk and assigns dummy names if
// dummy_names is set to true.
Signals AddPOs(Abc_Ntk_t *ntk, size_t n, bool dummy_names = true);

// Adds the specified amount of PIs to ntk and assigns dummy names if
// dummy_names is set to true.
Signals AddPIs(Abc_Ntk_t *ntk, size_t n, bool dummy_names = true);

// Returns the amount of non CO/CI-nodes in the subgraphs induced by the given
// signals.
size_t CountNodesFor(const Signals &signals);

// Sets the global BDDs of ntk's COs to the given ones.
void SetGlobalBDDs(Abc_Ntk_t *ntk, bdd::BDDs bdd);

// Returns true iff the global BDD vector of ntk is not nullptr
bool HasGlobalBDD(Abc_Ntk_t *ntk);

// Returns the global BDDs of ntk's COs.
bdd::BDDs GetGlobalBDD(Abc_Ntk_t *ntk);

} // namespace aig
} // namespace symmetrize
