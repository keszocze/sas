#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../includes.h"
#include "../wae_factors.h"

namespace symmetrize {
namespace bdd {

// Calculates the level (i.e. root has level 0 and sink level n + 1) of
// the given node for the current permutation of n variables in the manager.
int Level(DdNode *node, DdManager *manager, int n);

class BDD {

public:
  BDD(DdManager *manager, DdNode *node);
  BDD() = default;

  DdNode *Get() const { return node_.get(); }
  DdManager *GetManager() const { return manager_; }

  // Calculates the hamming distance between this BDD and the other. Requires
  // equal managers
  double HammingDistance(const BDD &other, size_t n) const;

  BDD operator!();
  friend bool operator==(const BDD &a, const BDD &b);

private:
  DdManager *manager_ = nullptr;
  std::shared_ptr<DdNode> node_ = nullptr;

};

// Helper wrapper for a vector of BDDs (components)
struct BDDs {

  // Combines t and f by selecting t[i] if s[i] == true, f[i] otherwise for
  // each component
  static BDDs Select(const BDDs &t, const BDDs &f, const std::vector<bool> &s);

  std::vector<BDD> components;

  // Returns the manager of the components
  DdManager *GetManager() const;

  // Counts the amount of nodes in all BDDs, including constants
  size_t Count() const;

  // Calculates the WAE with the given factors and variable count n
  double WAE(size_t n, const BDDs &other, const WAEFactorFunction &alpha) const;

  // Transfers all BDDs to the new manager
  BDDs Transfer(DdManager *new_mgr) const;

};

} // namespace bdd
} // namespace symmetrize
