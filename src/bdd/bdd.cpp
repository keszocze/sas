#include "bdd.h"

#include <stdexcept>

#include "../utils/vector.h"

namespace symmetrize {
namespace bdd {

int Level(DdNode *node, DdManager *manager, int n) {
  if (Cudd_Regular(node) == Cudd_ReadOne(manager))
    return n;
  return Cudd_ReadPerm(manager, Cudd_NodeReadIndex(node));
}

// +----------------------------------------------------------+
// |                           BDDs                           |
// +----------------------------------------------------------+

BDD::BDD(DdManager *manager, DdNode *node)
    : manager_(manager), node_(node, [manager](DdNode *node) {
        Cudd_RecursiveDeref(manager, node);
      }) {
  Cudd_Ref(node);
}

static double ComputeHammingDistance(DdManager *manager, DdNode *a, DdNode *b,
                                     int k, int n) {
  if (a == b)
    return 0;
  int a_level = Level(a, manager, n);
  int b_level = Level(b, manager, n);
  if (a_level > b_level)
    return ComputeHammingDistance(manager, b, a, k, n);
  // now a has first variable ( Label(a) <= Label(b) )
  if (a_level != k) {
    return pow(2, a_level - k) *
           ComputeHammingDistance(manager, a, b, a_level, n);
  }
  if (a_level == n) { // => a and b are constant nodes
    return a == b ? 0 : 1;
  }
  if (Cudd_IsComplement(a) != Cudd_IsComplement(b)) {
    return pow(2, n - k) -
           ComputeHammingDistance(manager, Cudd_Not(a), b, k, n);
  }
  DdNode *a_hi = Cudd_T(a);
  DdNode *a_lo = Cudd_E(a);
  DdNode *b_hi, *b_lo;
  if (b_level != k) {
    b_hi = b_lo = b;
  } else {
    b_hi = Cudd_T(b);
    b_lo = Cudd_E(b);
  }
  double hd = ComputeHammingDistance(manager, a_lo, b_lo, k + 1, n) +
              ComputeHammingDistance(manager, a_hi, b_hi, k + 1, n);
  return hd;
}

double BDD::HammingDistance(const BDD &other, size_t n) const {
  if (other.manager_ != manager_)
    throw std::invalid_argument("BDDs have to have the same manager");
  return ComputeHammingDistance(manager_, Get(), other.Get(), 0, n);
}

BDD BDD::operator!() { return {manager_, Cudd_Not(Get())}; }

bool operator==(const BDD &a, const BDD &b) { return a.Get() == b.Get(); }

// +----------------------------------------------------------+
// |                           BDDs                           |
// +----------------------------------------------------------+

BDDs BDDs::Select(const BDDs &t, const BDDs &f, const std::vector<bool> &s) {
  return {utils::Select<BDD>(t.components, f.components, s)};
}

DdManager *BDDs::GetManager() const {
  if (components.empty())
    throw std::invalid_argument(
        "BDDs with 0 components do not have a manager");

  for (auto next = components.begin(), drag = next++; next != components.end();
       next++, drag++) {
    if (next->GetManager() != drag->GetManager())
      throw std::invalid_argument(
          "different managers for different components of BDDs");
  }
  return components.front().GetManager();
}

static void CollectNodes(DdNode *root, std::unordered_set<DdNode *> &set) {
  root = Cudd_Regular(root);
  bool inserted = set.emplace(root).second;
  if (!inserted || Cudd_IsConstant(root))
    return;
  CollectNodes(Cudd_T(root), set);
  CollectNodes(Cudd_E(root), set);
}

size_t BDDs::Count() const {
  std::unordered_set<DdNode *> nodes;
  for (auto &bdd : components)
    CollectNodes(bdd.Get(), nodes);
  return nodes.size();
}

double BDDs::WAE(size_t n, const BDDs &other, const WAEFactorFunction &alpha) const {
  if (components.size() != other.components.size()) {
    throw std::invalid_argument("different m for other BDD");
  }
  double err = 0;
  for (size_t i = 0; i < components.size(); i++) {
    err += alpha(components.size(), i) *
           components[i].HammingDistance(other.components[i], n);
  }
  return err / pow(2, n);
}

BDDs BDDs::Transfer(DdManager *new_mgr) const {
  std::vector<BDD> bdds(components.size());
  for (size_t i = 0; i < bdds.size(); i++) {
    auto &c = components[i];
    bdds[i] = BDD(new_mgr, Cudd_bddTransfer(c.GetManager(), new_mgr, c.Get()));
  }
  return {bdds};
}

} // namespace bdd
} // namespace symmetrize
