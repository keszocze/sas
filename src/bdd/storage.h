#pragma once

#include "bdd.h"
#include <istream>
#include <ostream>

namespace symmetrize {
namespace bdd {

void Write(const bdd::BDDs &bdd, std::ostream &stream);
void Write(const bdd::BDDs &bdd, const std::string &filename);

bdd::BDDs Read(std::istream &stream, DdManager *manager = nullptr);
bdd::BDDs Read(const std::string &filename, DdManager *manager = nullptr);

} // namespace bdd
} // namespace symmetrize
