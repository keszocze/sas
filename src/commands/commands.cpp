#include "commands.h"

#include "common.h"
#include "gbdd.h"
#include "netgen.h"
#include "symmetrize.h"

namespace symmetrize {
namespace commands {

template <int CommandHandler(Abc_Frame_t *frame, int, char **)>
int CatchExceptions(Abc_Frame_t *frame, int argc, char **argv) {
  try {
    return CommandHandler(frame, argc, argv);
  } catch (std::exception &e) {
    Abc_Print(ABC_ERROR, "Exception caught: %s\n", e.what());
    return 1;
  } catch (...) {
    Abc_Print(ABC_ERROR, "Unknown exception caught\n");
    return 1;
  }
}

// runsc [command...]
int RepeatUntilNoSizeChange(Abc_Frame_t *frame, int argc, char **argv) {
  if (argc == 1) {
    Abc_Print(ABC_ERROR, "Usage: %s [command...]\n", argv[0]);
    return 1;
  }
  Abc_Ntk_t *ntk = Abc_FrameReadNtk(frame);
  if (ntk == nullptr) {
    Abc_Print(ABC_ERROR, "No network set.");
    return 1;
  }
  std::string command;
  for (int i = 1; i < argc; i++) {
    if (i != 1)
      command += " ";
    command += argv[i];
  }
  int old_size;
  int new_size = Abc_NtkNodeNum(ntk);
  int loops = 0;
  do {
    old_size = new_size;
    int code = Cmd_CommandExecute(frame, command.c_str());
    if (code) {
      Abc_Print(ABC_ERROR, "Command failed with code %i\n", code);
      return code;
    }
    new_size = Abc_NtkNodeNum(ntk);
    loops++;
  } while (old_size != new_size);
  Abc_Print(ABC_STANDARD, "Did %i loops total\n", loops);
  return 0;
}

void AddCommands(Abc_Frame_t *frame) {
  Cmd_CommandAdd(frame, "Symmetrize", "runsc", RepeatUntilNoSizeChange, 1);

  Cmd_CommandAdd(frame, "Symmetrize", "symmetrize",
                 CatchExceptions<CommandSymmetrize>, 1);

  Cmd_CommandAdd(frame, "Symmetrize", "netgen", CatchExceptions<CommandNetgen>,
                 1);

  Cmd_CommandAdd(frame, "Symmetrize", "gbdd_build",
                 CatchExceptions<CommandBuildGBDD>, 0);
  Cmd_CommandAdd(frame, "Symmetrize", "gbdd_store",
                 CatchExceptions<CommandStoreGBDD>, 0);
  Cmd_CommandAdd(frame, "Symmetrize", "gbdd_load",
                 CatchExceptions<CommandLoadGBDD>, 0);
}

} // namespace commands
} // namespace symmetrize
