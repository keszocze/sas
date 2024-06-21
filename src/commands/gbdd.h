#pragma once

#include "../includes.h"

namespace symmetrize {
namespace commands {

int CommandBuildGBDD(Abc_Frame_t *frame, int argc, char **argv);
int CommandStoreGBDD(Abc_Frame_t *frame, int argc, char **argv);
int CommandLoadGBDD(Abc_Frame_t *frame, int argc, char **argv);

} // namespace commands
} // namespace symmetrize
