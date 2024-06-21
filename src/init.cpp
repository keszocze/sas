#include "commands/commands.h"

namespace symmetrize {

static void Init(Abc_Frame_t *frame) { commands::AddCommands(frame); }

static void Destroy(Abc_Frame_t *frame) {}

Abc_FrameInitializer_t frame_initializer = {Init, Destroy};

struct Registrar {
  Registrar() { Abc_FrameAddInitializer(&frame_initializer); }
} static symm_registrar;

} // namespace symmetrize