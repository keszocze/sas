#include "network.h"

#include <bdd/extrab/extraBdd.h>
#include <stdexcept>
#include <unordered_set>

#include "../utils/maths.h"
#include "circuits.h"

namespace symmetrize {
namespace aig {

bool CleanupAndCheck(Abc_Ntk_t *ntk) {
  Abc_AigCleanup((Abc_Aig_t *)ntk->pManFunc);
  return Abc_NtkCheck(ntk);
}

static void AssignDummyNames(const Signals &signals, const std::string &prefix,
                             int begin) {
  for (int i = 0; i < signals.size(); i++) {
    std::string name = prefix + std::to_string(begin + i);
    // const cast here is fine, as it is just being copied in Abc_ObjAssignName
    Abc_ObjAssignName(signals[i], const_cast<char *>(name.c_str()), nullptr);
  }
}

Abc_Ntk_t *Create(size_t ni, size_t no, bool dummy_names) {
  Abc_Ntk_t *ntk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
  AddPIs(ntk, ni, dummy_names);
  AddPOs(ntk, no, dummy_names);
  return ntk;
}

void SetName(Abc_Ntk_t *ntk, const std::string &name) {
  Abc_NtkSetName(ntk, Extra_UtilStrsav(name.c_str()));
}

Signals GetPOs(Abc_Ntk_t *ntk) {
  Signals cos(Abc_NtkPoNum(ntk));
  for (int i = 0; i < cos.size(); i++) {
    cos[i] = Abc_NtkPo(ntk, i);
  }
  return cos;
}

Signals GetPIs(Abc_Ntk_t *ntk) {
  Signals pis(Abc_NtkPiNum(ntk));
  for (int i = 0; i < pis.size(); i++) {
    pis[i] = Abc_NtkPi(ntk, i);
  }
  return pis;
}

Signals AddPOs(Abc_Ntk_t *ntk, size_t n, bool dummy_names) {
  int n_before = Abc_NtkPoNum(ntk);
  Signals pos(n);
  for (size_t i = 0; i < n; i++) {
    pos[i] = Abc_NtkCreatePo(ntk);
  }
  if (dummy_names)
    AssignDummyNames(pos, "po", n_before);
  return pos;
}

void AddPOs(Abc_Ntk_t *ntk, const Signals &signals, bool dummy_names) {
  Signals pos = AddPOs(ntk, signals.size(), dummy_names);
  auto o_it = pos.begin();
  auto s_it = signals.begin();
  for (; o_it != pos.end(); o_it++, s_it++) {
    Abc_ObjAddFanin(*o_it, *s_it);
  }
}

Signals AddPIs(Abc_Ntk_t *ntk, size_t n, bool dummy_names) {
  int n_before = Abc_NtkPiNum(ntk);
  Signals pis(n);
  for (size_t i = 0; i < n; i++) {
    pis[i] = Abc_NtkCreatePi(ntk);
  }
  if (dummy_names)
    AssignDummyNames(pis, "pi", n_before);
  return pis;
}

// +----------------------------------------------------------+
// |                         COUNTING                         |
// +----------------------------------------------------------+

using NodeSet = std::unordered_set<Abc_Obj_t *>;

static void AddAll(Abc_Obj_t *node, NodeSet &set) {
  node = Abc_ObjRegular(node);
  if (Abc_ObjIsCi(node) || set.find(node) != set.end())
    return;
  if (!Abc_ObjIsCo(node))
    set.emplace(node);
  for (int i = 0; i < Abc_ObjFaninNum(node); i++) {
    AddAll(Abc_ObjFanin(node, i), set);
  }
}

size_t CountNodesFor(const Signals &signals) {
  NodeSet set;
  for (auto s : signals)
    AddAll(s, set);
  return set.size();
}

// +----------------------------------------------------------+
// |                        GLOBAL BDD                        |
// +----------------------------------------------------------+

bool HasGlobalBDD(Abc_Ntk_t *ntk) { return Abc_NtkGlobalBdd(ntk) != nullptr; }

bdd::BDDs GetGlobalBDD(Abc_Ntk_t *ntk) {
  if (Abc_NtkGlobalBdd(ntk) == nullptr) {
    throw std::invalid_argument("global BDD not set for network");
  }
  auto *manager = (DdManager *)Abc_NtkGlobalBddMan(ntk);
  std::vector<bdd::BDD> bdds;
  bdds.reserve(Abc_NtkCoNum(ntk));
  for (int i = 0; i < Abc_NtkCoNum(ntk); i++) {
    bdds.emplace_back(manager, (DdNode *)Abc_ObjGlobalBdd(Abc_NtkCo(ntk, i)));
  }
  return {bdds};
}

void SetGlobalBDDs(Abc_Ntk_t *ntk, bdd::BDDs bdd) {
  if (Abc_NtkGlobalBdd(ntk) != nullptr)
    throw std::invalid_argument("global BDD for network is already set");
  if (bdd.components.size() != Abc_NtkCoNum(ntk))
    throw std::invalid_argument("wrong amount of components in the given BDD");

  Vec_Att_t *global_bdds =
      Vec_AttAlloc(Abc_NtkObjNumMax(ntk) + 1, bdd.GetManager(),
                   (void (*)(void *))Extra_StopManager, NULL,
                   (void (*)(void *, void *))Cudd_RecursiveDeref);
  Vec_PtrWriteEntry(ntk->vAttrs, VEC_ATTR_GLOBAL_BDD, global_bdds);
  for (int i = 0; i < Abc_NtkCoNum(ntk); i++) {
    DdNode *node = bdd.components[i].Get();
    Abc_ObjSetGlobalBdd(Abc_NtkCo(ntk, i), node);
    Cudd_Ref(node);
  }
}

} // namespace aig
} // namespace symmetrize
