#include "componentwise.h"

#include <cmath>

#include "aig/network.h"
#include "aig/symmetric.h"

#include "bdd/ch.h"
#include "bdd/symmetric.h"

#include "utils/vector.h"

namespace symmetrize {

// +----------------------------------------------------------+
// |                      Profit Metrics                      |
// +----------------------------------------------------------+

std::map<std::string, ProfitMetric> ProfitMetrics::BY_NAME = {
    {"const", ProfitMetrics::Constant},
    {"aig", ProfitMetrics::AigSizeDifference},
    {"bdd", ProfitMetrics::BddSizeDifference}};

Profit ProfitMetrics::AigSizeDifference(ProfitMetricParameters p) {
  return (ssize_t)aig::CountNodesFor({p.f_i_po}) -
         (ssize_t)aig::CountNodesFor({p.f_tilde_i_po});
}
Profit ProfitMetrics::BddSizeDifference(ProfitMetricParameters p) {
  return (Profit)bdd::BDDs{{p.f_i_bdd}}.Count() -
         (Profit)bdd::BDDs{{p.f_tilde_i_bdd}}.Count();
}
Profit ProfitMetrics::Constant(ProfitMetricParameters) { return 1; }

// +----------------------------------------------------------+
// |                        Symmetrize                        |
// +----------------------------------------------------------+

utils::GreedyApproximateKnapsackSolver<double, Profit> DEFAULT_SOLVER =
    utils::GreedyApproximateKnapsackSolver<double, Profit>();

// Source: A. Bernasconi, V. Ciriani and T. Villa,
// "Exploiting Symmetrization and D-Reducibility for Approximate Logic
// Synthesis," in IEEE Transactions on Computers, vol. 71, no. 1, pp. 121-133,
// 1 Jan. 2022, doi: 10.1109/TC.2020.3043476.
static std::pair<ValueVector, ValueCount>
CalculateValueVector(const ValueCountsHW &T,
                     const BinomialCoefficients<ValueCount> &binomial) {
  size_t n = T.size() - 1;

  ValueVector vector(n + 1);
  ValueCount e = 0;
  const ValueCount *coefficients = binomial[n];
  for (int i = 0; i <= n; i++) {
    ValueCount zeros = coefficients[i] - T[i];
    if (T[i] > zeros) {
      vector[i] = true;
      e += zeros;
    } else {
      vector[i] = false;
      e += T[i];
    }
  }
  return {vector, e};
}

SymmetricFunction
CalculateSymmetricFunction(const std::vector<ValueCountsHW> &Ts,
                           const BinomialCoefficients<ValueCount> &binomial) {
  size_t m = Ts.size();
  std::vector<ValueVector> vvs(m);
  std::vector<ValueCount> hds(m);
  for (size_t i = 0; i < m; i++) {
    auto vv_hd = CalculateValueVector(Ts[i], binomial);
    vvs[i] = vv_hd.first;
    hds[i] = vv_hd.second;
  }
  return {.n = Ts.empty() ? 0 : Ts[0].size() - 1,
          .m = m,
          .components = vvs,
          .hamming_distances = hds};
}

void Symmetrize(ComponentwiseSymmetrizationParameters p) {
  // TODO: keep names
  if (p.ntk == nullptr || !Abc_NtkIsStrash(p.ntk)) {
    throw std::invalid_argument("given network is not an AIG");
  }
  if (!aig::HasGlobalBDD(p.ntk)) {
    throw std::invalid_argument("global BDDs for given network not set");
  }
  if (Abc_NtkCiNum(p.ntk) != Abc_NtkPiNum(p.ntk)) {
    throw std::invalid_argument("symmetrization does only support "
                                "combinatorial logic");
  }
  size_t n = Abc_NtkPiNum(p.ntk);
  size_t m = Abc_NtkPoNum(p.ntk);
  BinomialCoefficients<ValueCount> binomial(n);

  // Retrieve BDD from network and take ownership of manager
  std::unique_ptr<DdManager, void (*)(DdManager *)> mgr_ptr(
      (DdManager *)Abc_NtkGlobalBddMan(p.ntk), Cudd_Quit);
  auto f_bdd = aig::GetGlobalBDD(p.ntk);
  Abc_NtkFreeGlobalBdds(p.ntk, 0);

  size_t aig_size_before = Abc_NtkNodeNum(p.ntk);
  size_t bdd_size_before = f_bdd.Count();

  // Calculate nearest fully symmetric function f_tilde
  auto t_start = Abc_Clock();
  auto Ts = bdd::C_H(f_bdd, n, binomial);
  auto f_tilde = CalculateSymmetricFunction(Ts, binomial);
  Abc_PrintTime(ABC_VERBOSE, "t_symm", Abc_Clock() - t_start);

  // Add POs for f_tilde to AIG
  t_start = Abc_Clock();
  aig::AddSymmetricPOs(p.ntk, f_tilde);

  // Optimize
  if (p.frame && !p.optimization_command.empty()) {
    Cmd_CommandExecute(p.frame, p.optimization_command.c_str());
    p.ntk = Abc_FrameReadNtk(p.frame);
  }
  Abc_PrintTime(ABC_VERBOSE, "t_aig", Abc_Clock() - t_start);

  // Compute BDDs of f_tilde
  t_start = Abc_Clock();
  auto f_tilde_bdds = bdd::Create(f_bdd.GetManager(), f_tilde);
  Abc_PrintTime(ABC_VERBOSE, "t_bdd", Abc_Clock() - t_start);

  // Retrieve POs for components of f and f_tilde after potential optimization
  t_start = Abc_Clock();
  auto [f_i_aig, f_tilde_i_aig] = utils::Split(aig::GetPOs(p.ntk));

  // Compute profits
  std::vector<Profit> profits(m);
  for (size_t i = 0; i < m; i++) {
    profits[i] = p.profit_metric({.i = i,
                                  .f_i_po = f_i_aig[i],
                                  .f_tilde_i_po = f_tilde_i_aig[i],
                                  .f_i_bdd = f_bdd.components[i],
                                  .f_tilde_i_bdd = f_tilde_bdds.components[i]});
  }

  // Compute e_i
  std::vector<double> e_i = f_tilde.hamming_distances;
  double n_exp = exp2(n);
  for (size_t i = 0; i < m; i++) {
    e_i[i] *= p.factors(m, i) / n_exp;
  }

  // Solve knapsack problem
  auto [error, sigma] = p.knapsack_solver.Solve(e_i, profits, p.error_bound);

  // delete old POs and add new ones for f_hat
  auto f_hat_aig = utils::Select(f_tilde_i_aig, f_i_aig, sigma);
  for (auto &obj : f_hat_aig) {
    obj = Abc_ObjFanin(obj, 0);
  }
  while (Abc_NtkPoNum(p.ntk) != 0) {
    Abc_NtkDeleteObj(Abc_NtkPo(p.ntk, 0));
  }
  aig::AddPOs(p.ntk, f_hat_aig);

  // Compute new BDD and set
  auto f_hat_bdd = bdd::BDDs::Select(f_tilde_bdds, f_bdd, sigma);
  aig::SetGlobalBDDs(p.ntk, f_hat_bdd);
  mgr_ptr.release();
  Abc_PrintTime(ABC_VERBOSE, "t_select", Abc_Clock() - t_start);

  // cleanup and check
  if (!aig::CleanupAndCheck(p.ntk)) {
    throw std::logic_error("network check after symmetrization failed");
  }

  size_t aig_size_after = Abc_NtkNodeNum(p.ntk);
  size_t bdd_size_after = f_hat_bdd.Count();

  Abc_Print(ABC_STANDARD, "Symmetrization complete.\n", error);
  Abc_Print(ABC_STANDARD, "AIG size: %u -> %u (%.2f%%)\n", aig_size_before,
            aig_size_after,
            100.0 * ((double)aig_size_before - aig_size_after) /
                aig_size_before);
  Abc_Print(ABC_STANDARD, "BDD size: %u -> %u (%.2f%%)\n", bdd_size_before,
            bdd_size_after,
            100.0 * ((double)bdd_size_before - bdd_size_after) /
                bdd_size_before);
  size_t n_sigma = 0;
  for (auto e : sigma) {
    if (e) {
      n_sigma++;
    }
  }
  Abc_Print(ABC_STANDARD, "Selection: %s (%.2f%% of components)\n",
            tt::ToString(sigma).c_str(), 100.0 * n_sigma / sigma.size());
  Abc_Print(ABC_STANDARD, "Total error: %.2f\n", error);
}

} // namespace symmetrize
