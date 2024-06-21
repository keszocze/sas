#include "ch.h"

#include <unordered_map>

namespace symmetrize {
namespace bdd {

// TODO: caching

using Bin = const BinomialCoefficients<ValueCount> &;
using Cache = std::unordered_map<DdNode *, ValueCountsHW>;

static ValueCountsHW ComplementValueCounts(ValueCountsHW v, Bin bin) {
  for (size_t i = 0; i < v.size(); i++) {
    v[i] = bin.at(v.size() - 1, i) - v[i];
  }
  return v;
}

static ValueCountsHW Combine(const ValueCountsHW &t, ValueCountsHW e) {
  e.resize(e.size() + 1);
  auto t_it = t.begin();
  auto e_it = ++e.begin();
  while (t_it != t.end()) {
    *(e_it++) += *(t_it++);
  }
  return e;
}

static ValueCountsHW Expand(ValueCountsHW v, int d, Bin bin) {
  if (d == 0) return v;
  ValueCountsHW res(v.size() + d);
  for (size_t i = 0; i <= d; i++) {
    for (size_t j = 0; j < v.size(); j++) {
      res[i + j] = res[i + j] + bin.at(d, i) * v[j];
    }
  }
  return res;
}

static ValueCountsHW Count(DdNode *b, DdManager *mgr, int l, int n,
                           Cache &cache, Bin bin) {
  if (Cudd_IsComplement(b)) {
    auto v = Count(Cudd_Regular(b), mgr, l, n, cache, bin);
    return ComplementValueCounts(std::move(v), bin);
  }
  int l_curr = Level(b, mgr, n);
  int d = l_curr - l;
  if (Cudd_IsConstant(b)) {
    ValueCountsHW v = {(ValueCount)(b == Cudd_ReadOne(mgr))};
    return Expand(std::move(v), d, bin);
  }
  ValueCountsHW node_ch;
  auto cache_entry = cache.find(b);
  if (cache_entry != cache.end()) {
    node_ch = cache_entry->second;
  } else {
    auto t = Count(Cudd_T(b), mgr, l_curr + 1, n, cache, bin);
    auto e = Count(Cudd_E(b), mgr, l_curr + 1, n, cache, bin);
    node_ch = Combine(t, std::move(e));
    cache.emplace(b, node_ch);
  }
  return Expand(node_ch, d, bin);
}

std::vector<ValueCountsHW> C_H(const BDDs &bdds, int n, Bin bin) {
  Cache cache;
  std::vector<ValueCountsHW> result;
  result.reserve(bdds.components.size());
  for (const BDD &bdd : bdds.components) {
    result.push_back(Count(bdd.Get(), bdd.GetManager(), 0, n, cache, bin));
  }
  return result;
}

} // namespace bdd
} // namespace symmetrize
