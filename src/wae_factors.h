#pragma once

#include <functional>
#include <string>
#include <unordered_map>

namespace symmetrize {

// given m and i, returns a_i for a function f: B^n -> B^m
using WAEFactorFunction = std::function<double(size_t m, size_t i)>;

struct WAEFactors {
  static const std::unordered_map<std::string, WAEFactorFunction> BY_NAME;

  static double ErrorRate(size_t m, size_t i);
  static double AWAE(size_t m, size_t i);
  static double NAWAE(size_t m, size_t i);
};

} // namespace symmetrize