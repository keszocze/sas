#include "netgen.h"

#include <algorithm>

#include "../aig/circuits.h"
#include "../aig/network.h"
#include "common.h"

namespace symmetrize {
namespace commands {

const char *USAGE = "netgen [adder/multiplier] [bits]\n"
                    "netgen [mac] [bits] [n pairs]\n"
                    "netgen [asymmetric] [n PIs] [n POs]\n";

static void PrintUsage() { Abc_Print(ABC_ERROR, USAGE); }

std::vector<std::string> NET_TYPES = {"adder", "multiplier", "mac",
                                      "asymmetric"};
std::vector<int> ARG_COUNT = {3, 3, 4, 4};

int CommandNetgen(Abc_Frame_t *frame, int argc, char **argv) {
  if (argc < 2) {
    PrintUsage();
    return 1;
  }
  size_t typeId =
      std::distance(NET_TYPES.begin(),
                    std::find(NET_TYPES.begin(), NET_TYPES.end(), argv[1]));
  size_t n;
  if (typeId == NET_TYPES.size() || argc != ARG_COUNT[typeId] ||
      !ToSize(argv[2], n)) {
    PrintUsage();
    return 1;
  }
  Abc_Ntk_t *ntk = aig::Create();
  aig::Signals out;
  if (typeId == 0) { // Adder
    auto a = aig::AddPIs(ntk, n);
    auto b = aig::AddPIs(ntk, n);
    out = aig::Adder(ntk, a, b);
    aig::SetName(ntk, "add" + std::to_string(n));
  } else if (typeId == 1) { // Multiplier
    auto a = aig::AddPIs(ntk, n);
    auto b = aig::AddPIs(ntk, n);
    out = aig::Multiplier(ntk, a, b);
    aig::SetName(ntk, "multiply" + std::to_string(n));
  } else {
    size_t m;
    if (!ToSize(argv[3], m)) {
      PrintUsage();
      return 1;
    }
    if (typeId == 2) { // MAC
      aig::NumberPairs pairs;
      for (size_t i = 0; i < m; i++) {
        pairs.emplace_back(aig::AddPIs(ntk, n), aig::AddPIs(ntk, n));
      }
      out = aig::MAC(ntk, pairs);
      aig::SetName(ntk, "mac " + std::to_string(m) + " x multiply" +
                            std::to_string(n));
    } else if (typeId == 3) { // Asymmetric
      out.reserve(m);
      auto in = aig::AddPIs(ntk, n);
      for (size_t i = 0; i < m; i++) {
        out.emplace_back(aig::RandomMaximallyAsymmetric(ntk, in));
      }
      aig::SetName(ntk, "rand max asymm B^" + std::to_string(n) + " -> B^" +
                            std::to_string(m));
    } else {
      PrintUsage();
      return 1;
    }
  }
  aig::AddPOs(ntk, out);
  if (!aig::CleanupAndCheck(ntk)) {
    Abc_Print(ABC_ERROR, "Network sanity check failed\n");
    return 1;
  }
  Abc_FrameReplaceCurrentNetwork(frame, ntk);
  return 0;
}

} // namespace commands
} // namespace symmetrize
