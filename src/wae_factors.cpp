#include "wae_factors.h"

#include <vector>

namespace symmetrize {

static std::vector<double> powers_of_two(1, 1);

static void EnsurePowersArePresent(size_t idx) {
  powers_of_two.reserve(idx + 1);
  while (powers_of_two.size() <= idx) {
    powers_of_two.emplace_back(powers_of_two.back() * 2);
  }
}

const std::unordered_map<std::string, WAEFactorFunction> WAEFactors::BY_NAME{
    {"er", WAEFactors::ErrorRate},
    {"awae", WAEFactors::AWAE},
    {"nawae", WAEFactors::NAWAE}};

double WAEFactors::ErrorRate(size_t m, size_t i) { return 100 / (double)m; }

double WAEFactors::AWAE(size_t m, size_t i) {
  EnsurePowersArePresent(i);
  return powers_of_two[i];
}

double WAEFactors::NAWAE(size_t m, size_t i) {
  EnsurePowersArePresent(m);
  return 100 * powers_of_two[i] / (powers_of_two[m] - 1);
}

} // namespace symmetrize