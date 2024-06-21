#include "symmetric.h"

namespace symmetrize {
namespace bdd {

// Source: A. Bernasconi, V. Ciriani and T. Villa,
// "Exploiting Symmetrization and D-Reducibility for Approximate Logic
// Synthesis," in IEEE Transactions on Computers, vol. 71, no. 1, pp. 121-133,
// 1 Jan. 2022, doi: 10.1109/TC.2020.3043476.

class MMatrix {

public:
  MMatrix(DdManager *mgr, int n);
  MMatrix(DdManager *mgr, std::vector<int> vars);
  ~MMatrix() { clear(); }

  void clear() {
    for (auto &node : m)
      Cudd_RecursiveDeref(mgr, node);
    m.clear();
  }

  DdNode **operator[](size_t row) { return m.data() + (row * width); }

private:
  DdManager *mgr;
  size_t width;
  std::vector<DdNode *> m;
};

static std::vector<int> Range(int n) {
  std::vector<int> res(n);
  for (int i = 0; i < res.size(); i++) {
    res[i] = i;
  }
  return res;
}

MMatrix::MMatrix(DdManager *mgr, int n) : MMatrix(mgr, Range(n)) {}

MMatrix::MMatrix(DdManager *mgr, std::vector<int> vars)
    : mgr(mgr), width(vars.size() + 2), m(width * (vars.size() + 1)) {
  MMatrix &M = *this;
  size_t n = vars.size();

  M[0][1] = Cudd_ReadOne(mgr);
  Cudd_Ref(M[0][1]);
  for (size_t i = 0; i <= n; i++) {
    DdNode *node = Cudd_ReadLogicZero(mgr);
    Cudd_Ref(node);
    M[i][0] = node;
  }
  for (size_t i = 0; i <= n; i++) {
    for (size_t j = i + 2; j <= n + 1; j++) {
      DdNode *node = Cudd_ReadLogicZero(mgr);
      Cudd_Ref(node);
      M[i][j] = node;
    }
  }
  for (size_t i = 1; i <= n; i++) {
    for (size_t j = 1; j <= i + 1; j++) {
      DdNode *node = Cudd_bddIte(mgr, Cudd_bddIthVar(mgr, vars[i - 1]),
                                 M[i - 1][j - 1], M[i - 1][j]);
      Cudd_Ref(node);
      M[i][j] = node;
    }
  }
}

static DdNode *ValueVectorToBDD(DdManager *mgr, const ValueVector &vector,
                                MMatrix &m) {
  size_t n = vector.size() - 1;
  DdNode *res = Cudd_ReadLogicZero(mgr);
  Cudd_Ref(res);
  for (size_t i = 0; i <= n; i++) {
    if (vector[i]) {
      DdNode *new_res = Cudd_bddOr(mgr, res, m[n][i + 1]);
      Cudd_Ref(new_res);
      Cudd_RecursiveDeref(mgr, res);
      res = new_res;
    }
  }
  Cudd_Deref(res);
  return res;
}

BDDs Create(DdManager *manager, const SymmetricFunction &f) {
  size_t m = f.components.size();
  std::vector<BDD> bdds;
  bdds.reserve(m);
  MMatrix M(manager, f.n);
  for (auto &component : f.components) {
    bdds.emplace_back(manager, ValueVectorToBDD(manager, component, M));
  }
  return {bdds};
}

} // namespace bdd
} // namespace symmetrize
