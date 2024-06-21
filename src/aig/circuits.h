#pragma once

/**
 * Contains functions for creating various circuits within an AIG
 */

#include "../includes.h"
#include <vector>

#include "network.h"

namespace symmetrize {
namespace aig {

// Vector of AIG nodes that shall be treated as a number input for a circuit.
// The i-th entry of the vector naturally corresponds to the digit with value
// 2^i
using Number = Signals;

// Creates a circuit for a randomly generated maximally asymmetric function
// see: S. Nagayama, T. Sasao and J. T. Butler,
// "On Decision Diagrams for Maximally Asymmetric Functions," 2022
// IEEE 52nd International Symposium on Multiple-Valued Logic (ISMVL),
// Dallas, TX, USA, 2022, pp. 164-169, doi: 10.1109/ISMVL52857.2022.00032
//
// Assumes ntk is an AIG
Signal RandomMaximallyAsymmetric(Abc_Ntk_t *ntk, const Signals &inputs);

// Creates a LUT for the given truth table returning the Signal corresponding to
// "table[idx]"
//
// Assumes ntk is an AIG
Signal MuxLUT(Abc_Ntk_t *ntk, const TruthTable &table, const Number &idx);

using NumberPair = std::pair<Number, Number>;
using NumberPairs = std::vector<NumberPair>;

struct AdderOutputs {
  Signal sum;
  Signal carry;
  Number ToNumber() const { return {sum, carry}; }
};

// Returns the output signals of a new full adder in ntk with the given inputs.
//
// Assumes ntk is an AIG
AdderOutputs FullAdder(Abc_Ntk_t *ntk, Signal a, Signal b, Signal c);

// Returns the output signals of a new half adder in ntk with the given inputs.
//
// Assumes ntk is an AIG
AdderOutputs HalfAdder(Abc_Ntk_t *ntk, Signal a, Signal b);

// Creates a ripple carry adder in ntk that computes a + b
//
// Assumes ntk is an AIG
Number Adder(Abc_Ntk_t *ntk, const Number &a, const Number &b);

// Creates a ripple carry adder in ntk that computes a + b + cin
//
// Assumes ntk is an AIG
Number Adder(Abc_Ntk_t *ntk, const Number &a, const Number &b, Signal cin);

// Creates a bit counter in ntk that computes the number of ones in the given
// signals
// see: E. E. Swartzlander, "Parallel Counters,"
// in IEEE Transactions on Computers, vol. C-22, no. 11, pp. 1021-1024,
// Nov. 1973, doi: 10.1109/T-C.1973.223639.
//
// Assumes ntk is an AIG
Number BitCounter(Abc_Ntk_t *ntk, const Signals &signals);

// Creates an array multiplier in ntk that calculates a * b
//
// Assumes ntk is an AIG
Number Multiplier(Abc_Ntk_t *ntk, const Number &a, const Number &b);

// Creates a multiply-accumulate circuit in ntk that computes the products of
// each number pair and sums up the results
//
// Assumes ntk is an AIG
Number MAC(Abc_Ntk_t *ntk, const NumberPairs &pairs);

} // namespace aig
} // namespace symmetrize
