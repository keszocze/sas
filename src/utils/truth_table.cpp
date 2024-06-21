#include "truth_table.h"

#include "../utils/maths.h"
#include "array.h"
#include <stdexcept>

namespace symmetrize {
namespace tt {

std::string ToString(const TruthTable &table, bool fill_dc) {
  size_t target_length = fill_dc ? 1 << Log2(table.size()) : table.size();
  std::string res;
  res.reserve(target_length);
  for (auto e : table)
    res.push_back(e ? '1' : '0');
  while (res.size() != target_length) {
    res.push_back('-');
  }
  return res;
}

using SubTable = utils::Array<TruthValue, TruthTable::const_iterator>;

static void AppendOptimally(const TruthTable &T, TruthTable &target,
                            size_t tau_begin, size_t tau_order) {
  size_t tau_size = 1 << tau_order;
  SubTable tau = SubTable(T).subarray(tau_begin);
  if (tau.size() == tau_size)
    return; // nothing to append

  if (tau_order == 0) {
    target.emplace_back(tau.begin()[0]);
    return;
  }

  // Right subtable only dont cares
  size_t subtable_size = tau_size / 2;
  if (tau.size() <= subtable_size) {
    AppendOptimally(T, target, tau_begin, tau_order - 1);
    SubTable new_tau_l = SubTable(target).subarray(tau_begin);
    for (auto e : new_tau_l)
      target.emplace_back(e);
    return;
  }

  // Left subtable starts with right subtable
  SubTable tau_l = tau.first(subtable_size);
  SubTable tau_r = tau.subarray(subtable_size);
  if (tau_l.starts_with(tau_r)) {
    for (size_t i = tau_r.size(); i < subtable_size; i++) {
      target.emplace_back(tau_l.begin()[i]);
    }
    return;
  }

  // Fully specified subtable in T of same order as tau that starts with tau
  size_t st_begin = 0;
  while (T.size() - st_begin >= tau_size) {
    SubTable st = SubTable(T).subarray(st_begin, tau_size);
    if (st.starts_with(tau)) {
      for (size_t i = tau.size(); i < tau_size; i++) {
        target.emplace_back(st.begin()[i]);
      }
      return;
    }
    st_begin += tau_size;
  }

  AppendOptimally(T, target, tau_begin + subtable_size, tau_order - 1);
}

TruthTable FillMinBeads(const TruthTable &table) {
  if (table.empty()) {
    throw std::invalid_argument(
        "table has to have at least one specified value");
  }
  TruthTable res = table;
  AppendOptimally(table, res, 0, Log2(table.size()));
  return res;
}

} // namespace tt
} // namespace symmetrize
