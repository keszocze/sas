#pragma once

#include <functional>

#include "includes.h"
#include "wae_factors.h"

#include "bdd/bdd.h"

#include "utils/knapsack.h"
#include "utils/maths.h"

namespace symmetrize {

using Profit = ssize_t;

struct ProfitMetricParameters {
  size_t i;
  Abc_Obj_t *f_i_po;
  Abc_Obj_t *f_tilde_i_po;
  bdd::BDD &f_i_bdd;
  bdd::BDD &f_tilde_i_bdd;
};

// Given i, o and bdd, returns the profit of selecting the component f_i with
// PO o and BDD bdd
using ProfitMetric = std::function<Profit(ProfitMetricParameters)>;

extern utils::GreedyApproximateKnapsackSolver<double, Profit> DEFAULT_SOLVER;

struct ProfitMetrics {
  static std::map<std::string, ProfitMetric> BY_NAME;

  static Profit AigSizeDifference(ProfitMetricParameters p);
  static Profit BddSizeDifference(ProfitMetricParameters p);
  static Profit Constant(ProfitMetricParameters p);
};

struct ComponentwiseSymmetrizationParameters {
  Abc_Frame_t *frame = nullptr;
  Abc_Ntk_t *ntk = nullptr;

  WAEFactorFunction factors;
  double error_bound = 0;

  ProfitMetric profit_metric;
  const utils::KnapsackSolver<double, Profit> &knapsack_solver = DEFAULT_SOLVER;

  std::string optimization_command;
};

SymmetricFunction
CalculateSymmetricFunction(const std::vector<ValueCountsHW> &Ts,
                           const BinomialCoefficients<ValueCount> &binomial);

void Symmetrize(ComponentwiseSymmetrizationParameters parameters);

} // namespace symmetrize
