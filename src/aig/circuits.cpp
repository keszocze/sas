#include "circuits.h"

#include <array>
#include <random>
#include <stdexcept>

#include "../utils/maths.h"

#define AIG(ntk) ((Abc_Aig_t *)ntk->pManFunc)

namespace symmetrize {
namespace aig {

// +----------------------------------------------------------+
// |          RANDOM MAXIMALLY ASYMMETRIC FUNCTIONS           |
// +----------------------------------------------------------+
struct RMASState {
  using ValueCount = size_t;

  Abc_Ntk_t *ntk;
  BinomialCoefficients<ValueCount> coefficients;
  const std::vector<Abc_Obj_t *> &inputs;
  std::vector<std::array<ValueCount, 2>> classes;
  std::vector<bool> current_in;
  std::random_device dev;
  std::mt19937 rng;
  std::uniform_int_distribution<int> distr;

  RMASState(Abc_Ntk_t *ntk, const std::vector<Abc_Obj_t *> &inputs)
      : ntk(ntk), coefficients(inputs.size()), inputs(inputs),
        classes(inputs.size() + 1), dev(), rng(dev()), distr(0, 1) {}
};

static Signal RandomMaximallyAsymmetric(RMASState &s) {
  // see: S. Nagayama, T. Sasao and J. T. Butler,
  // "On Decision Diagrams for Maximally Asymmetric Functions," 2022
  // IEEE 52nd International Symposium on Multiple-Valued Logic (ISMVL),
  // Dallas, TX, USA, 2022, pp. 164-169, doi: 10.1109/ISMVL52857.2022.00032
  using ValueCount = RMASState::ValueCount;
  size_t n = s.inputs.size();
  if (s.current_in.size() == s.inputs.size()) {
    size_t hw = 0;
    for (auto v : s.current_in)
      hw += v ? 1 : 0;
    std::array<ValueCount, 2> &values = s.classes[hw];
    ValueCount max = (s.coefficients.at(n, hw) + 1) / 2;
    bool set_one = false;
    if (values[0] < max && values[1] < max) {
      set_one = s.distr(s.rng) == 1;
    } else if (values[1] < max) {
      set_one = true;
    }
    if (set_one) {
      values[1]++;
      Abc_Obj_t *product = Abc_AigConst1(s.ntk);
      for (size_t i = 0; i < n; i++) {
        Abc_Obj_t *var = s.inputs[i];
        if (!s.current_in[i])
          var = Abc_ObjNot(var);
        product = Abc_AigAnd(AIG(s.ntk), product, var);
      }
      return product;
    } else {
      values[0]++;
      return Abc_ObjNot(Abc_AigConst1(s.ntk));
    }
  } else {
    s.current_in.emplace_back(false);
    Abc_Obj_t *zero = RandomMaximallyAsymmetric(s);
    s.current_in.back() = true;
    Abc_Obj_t *one = RandomMaximallyAsymmetric(s);
    s.current_in.pop_back();
    return Abc_AigOr(AIG(s.ntk), zero, one);
  }
}

Signal RandomMaximallyAsymmetric(Abc_Ntk_t *ntk, const Signals &inputs) {
  RMASState state(ntk, inputs);
  return RandomMaximallyAsymmetric(state);
}

// +----------------------------------------------------------+
// |                           LUT                            |
// +----------------------------------------------------------+
template <typename TTIt, typename SIt>
Signal MuxLUT(Abc_Ntk_t *ntk, TTIt tt_from, TTIt tt_to, SIt s_from, SIt s_to) {
  size_t n = std::distance(tt_from, tt_to);
  if (n == 1) {
    Signal one = Abc_AigConst1(ntk);
    return *tt_from ? one : Abc_ObjNot(one);
  }
  Signal ctrl = *(--s_to);
  return Abc_AigMux(AIG(ntk), ctrl,
                    MuxLUT(ntk, tt_from + n / 2, tt_to, s_from, s_to),
                    MuxLUT(ntk, tt_from, tt_from + n / 2, s_from, s_to));
}

Signal MuxLUT(Abc_Ntk_t *ntk, const TruthTable &table,
              const Number &table_index) {
  if (!IsPow2(table.size()) || Log2(table.size()) != table_index.size()) {
    throw std::invalid_argument(
        "truth table has to be complete and of correct size for inputs");
  }
  return MuxLUT(ntk, table.begin(), table.end(), table_index.begin(),
                table_index.end());
}

// +----------------------------------------------------------+
// |                        ARITHMETIC                        |
// +----------------------------------------------------------+
AdderOutputs HalfAdder(Abc_Ntk_t *ntk, Signal a, Signal b) {
  return {Abc_AigXor(AIG(ntk), a, b), Abc_AigAnd(AIG(ntk), a, b)};
}

AdderOutputs FullAdder(Abc_Ntk_t *ntk, Signal a, Signal b, Signal c) {
  AdderOutputs add1 = HalfAdder(ntk, a, b);
  AdderOutputs add2 = HalfAdder(ntk, c, add1.sum);
  return {add2.sum, Abc_AigOr(AIG(ntk), add1.carry, add2.carry)};
}

Number Adder(Abc_Ntk_t *ntk, const Number &a, const Number &b) {
  return Adder(ntk, a, b, Abc_ObjNot(Abc_AigConst1(ntk)));
}

Number Adder(Abc_Ntk_t *ntk, const Number &a, const Number &b, Signal cin) {
  if (a.size() < b.size())
    return Adder(ntk, b, a, cin);
  if (a.empty())
    return {};
  std::vector<Abc_Obj_t *> result;
  result.reserve(a.size() + 1);

  auto it_a = a.begin();
  auto it_b = b.begin();
  while (it_b != b.end()) {
    AdderOutputs adder = FullAdder(ntk, *it_a++, *it_b++, cin);
    result.push_back(adder.sum);
    cin = adder.carry;
  }
  while (it_a != a.end()) {
    AdderOutputs adder = HalfAdder(ntk, *it_a++, cin);
    result.push_back(adder.sum);
    cin = adder.carry;
  }
  result.push_back(cin);
  return result;
}

template <typename It>
static Number BitCounter(Abc_Ntk_t *ntk, It from, It to) {
  // see: E. E. Swartzlander, "Parallel Counters,"
  // in IEEE Transactions on Computers, vol. C-22, no. 11, pp. 1021-1024,
  // Nov. 1973, doi: 10.1109/T-C.1973.223639.
  size_t n = std::distance(from, to);
  if (n == 1) {
    return {*from};
  } else if (n == 2) {
    return HalfAdder(ntk, *from++, *from++).ToNumber();
  } else if (n == 3) {
    return FullAdder(ntk, *from++, *from++, *from++).ToNumber();
  }
  Signal cin = *(--to);
  n--;
  Number a = BitCounter(ntk, from, from + n / 2);
  Number b = BitCounter(ntk, from + n / 2, to);
  return Adder(ntk, a, b, cin);
}

Number BitCounter(Abc_Ntk_t *ntk, const Signals &signals) {
  return BitCounter(ntk, signals.begin(), signals.end());
}

static AdderOutputs MultiplierCell(Abc_Ntk_t *ntk, Signal s, Signal a, Signal b,
                                   Signal c) {
  Signal ab = Abc_AigAnd(AIG(ntk), a, b);
  return FullAdder(ntk, s, ab, c);
}

Number Multiplier(Abc_Ntk_t *ntk, const Number &as, const Number &bs) {
  Abc_Obj_t *zero = Abc_ObjNot(Abc_AigConst1(ntk));
  if (as.size() < bs.size()) {
    return Multiplier(ntk, bs, as);
  }
  // a.size() >= b.size()
  std::vector<Abc_Obj_t *> result;
  result.reserve(as.size() + bs.size() + 1);

  auto b_it = bs.begin();
  std::vector<AdderOutputs> previous;
  previous.reserve(as.size());

  // Generate first level
  {
    Abc_Obj_t *carry = zero;
    for (auto &a : as) {
      AdderOutputs adder = MultiplierCell(ntk, zero, a, *b_it, carry);
      carry = adder.carry;
      previous.emplace_back(adder);
    }
    b_it++;
  }

  // Now all the other levels
  while (b_it != bs.end()) {
    Abc_Obj_t *b = *b_it;
    result.emplace_back(previous[0].sum);
    Abc_Obj_t *carry = zero;
    for (size_t ai = 0; ai < as.size(); ai++) {
      Abc_Obj_t *s =
          ai == as.size() - 1 ? previous[ai].carry : previous[ai + 1].sum;
      AdderOutputs cell = MultiplierCell(ntk, s, as[ai], b, carry);
      previous[ai] = cell;
      carry = cell.carry;
    }
    b_it++;
  }

  for (auto &cells : previous) {
    result.emplace_back(cells.sum);
  }
  result.emplace_back(previous.back().carry);
  return result;
}

template <typename It> Number MAC(Abc_Ntk_t *ntk, It from, It to) {
  size_t n = std::distance(from, to);
  if (n == 1) {
    return Multiplier(ntk, from->first, from->second);
  } else {
    return Adder(ntk, MAC(ntk, from, from + n / 2), MAC(ntk, from + n / 2, to));
  }
}

Number MAC(Abc_Ntk_t *ntk, const NumberPairs &pairs) {
  return MAC(ntk, pairs.begin(), pairs.end());
}

} // namespace aig
} // namespace symmetrize