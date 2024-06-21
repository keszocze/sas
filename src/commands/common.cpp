#include "common.h"

#include <stdexcept>

namespace symmetrize {
namespace commands {

bool ToSize(const char *txt, size_t &res) {
  char *end;
  errno = 0;
  res = strtoul(txt, &end, 10);
  if (errno || end == txt) {
    return false;
  }
  return true;
}

bool ToDouble(const char *txt, double &res) {
  char *end;
  errno = 0;
  res = strtod(txt, &end);
  if (errno || end == txt) {
    return false;
  }
  return true;
}

} // namespace commands
} // namespace symmetrize
