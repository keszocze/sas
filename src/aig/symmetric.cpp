#include "symmetric.h"

#include <stdexcept>

#include "circuits.h"

#include "../utils/truth_table.h"

namespace symmetrize {
namespace aig {

Signals AddSymmetricPOs(Abc_Ntk_t *ntk, const SymmetricFunction &f,
                        const Signals &inputs) {
  Number sum = BitCounter(ntk, inputs);
  Signals out(f.components.size());
  for (size_t i = 0; i < f.components.size(); i++) {
    TruthTable tt = f.components[i];
    tt = tt::FillMinBeads(tt);
    out[i] = MuxLUT(ntk, tt, sum);
  }
  AddPOs(ntk, out);
  Abc_AigCleanup((Abc_Aig_t *)ntk->pManFunc);
  return out;
}

Signals AddSymmetricPOs(Abc_Ntk_t *ntk, const SymmetricFunction &f) {
  if (f.n != Abc_NtkPiNum(ntk)) {
    throw std::invalid_argument("invalid amount of network inputs");
  }
  return AddSymmetricPOs(ntk, f, GetPIs(ntk));
}

Abc_Ntk_t *CreateSymmetricNetwork(const SymmetricFunction &f) {
  Abc_Ntk_t *ntk = Create(f.n);
  AddSymmetricPOs(ntk, f);
  return ntk;
}

} // namespace aig
} // namespace symmetrize
