#include "gbdd.h"

#include "../aig/network.h"
#include "../bdd/storage.h"

namespace symmetrize {
namespace commands {

const char *USAGE_BUILD = "gbdd_build [dynamic reordering: 0/1]\n";

int CommandBuildGBDD(Abc_Frame_t *frame, int argc, char **argv) {
  if (argc != 2) {
    Abc_Print(ABC_ERROR, USAGE_BUILD);
    return 1;
  }
  std::string reorder_str = std::string(argv[1]);
  bool reorder;
  if (reorder_str == "0") {
    reorder = false;
  } else if (reorder_str == "1") {
    reorder = true;
  } else {
    Abc_Print(ABC_ERROR, USAGE_BUILD);
    return 1;
  }

  Abc_Ntk_t *ntk = Abc_FrameReadNtk(frame);
  if (aig::HasGlobalBDD(ntk)) {
    Abc_Print(ABC_ERROR, "Global BDD is already set.");
    return 1;
  }
  Abc_NtkBuildGlobalBdds(ntk, std::numeric_limits<int>::max(), 1, reorder, 0,
                         1);
  Abc_Print(ABC_STANDARD, "Global BDDs built successfully.\n");
  Abc_Print(ABC_STANDARD, "Node count: %u\n", aig::GetGlobalBDD(ntk).Count());
  return 0;
}

const char *USAGE_STORE = "gbdd_store [filename]\n";

int CommandStoreGBDD(Abc_Frame_t *frame, int argc, char **argv) {
  if (argc != 2) {
    Abc_Print(ABC_ERROR, USAGE_STORE);
    return 1;
  }
  bdd::BDDs bdd = aig::GetGlobalBDD(Abc_FrameReadNtk(frame));
  bdd::Write(bdd, argv[1]);
  return 0;
}

const char *USAGE_LOAD = "gbdd_load [filename]\n";

int CommandLoadGBDD(Abc_Frame_t *frame, int argc, char **argv) {
  if (argc != 2) {
    Abc_Print(ABC_ERROR, USAGE_LOAD);
    return 1;
  }
  Abc_Ntk_t *ntk = Abc_FrameReadNtk(frame);
  bdd::BDDs bdd = bdd::Read(argv[1]);
  if (bdd.components.size() != Abc_NtkCoNum(ntk)) {
    DdManager *manager = bdd.GetManager();
    bdd.components = {};
    Cudd_Quit(manager);
    Abc_Print(ABC_ERROR, "Wrong number of roots.\n");
    return 1;
  }
  aig::SetGlobalBDDs(ntk, bdd);
  Abc_Print(ABC_STANDARD, "Success.\n");
  return 0;
}

} // namespace commands
} // namespace symmetrize
