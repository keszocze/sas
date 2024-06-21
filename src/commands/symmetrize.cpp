#include "symmetrize.h"

#include "common.h"

#include "../componentwise.h"

namespace symmetrize {
namespace commands {

static const char *USAGE =
    "symmetrize [error: er/awae/nawae] [error bound] [profit: const/aig/bdd] "
    "<optimization command>\n";

// symmetrize [error: er/awae/nawae] [error bound] [profit: const/aig/bdd]
//            <optimization command>
int CommandSymmetrize(Abc_Frame_t *frame, int argc, char **argv) {
  if (argc < 4) {
    Abc_Print(ABC_ERROR, USAGE);
    return 1;
  }
  ComponentwiseSymmetrizationParameters param;
  param.frame = frame;
  param.ntk = Abc_FrameReadNtk(frame);

  if (argc > 4) {
    param.optimization_command = argv[4];
  }

  {
    auto it = WAEFactors::BY_NAME.find(argv[1]);
    if (it == WAEFactors::BY_NAME.end()) {
      Abc_Print(ABC_ERROR, USAGE);
      return 1;
    }
    param.factors = it->second;
  }

  if (!ToDouble(argv[2], param.error_bound)) {
    Abc_Print(ABC_ERROR, USAGE);
    return 1;
  }

  {
    auto it = ProfitMetrics::BY_NAME.find(argv[3]);
    if (it == ProfitMetrics::BY_NAME.end()) {
      Abc_Print(ABC_ERROR, USAGE);
      return 1;
    }
    param.profit_metric = it->second;
  }

  Symmetrize(param);
  return 0;
}

} // namespace commands
} // namespace symmetrize
