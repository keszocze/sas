#pragma once

#include <limits>
#include <map>
#include <stdexcept>

namespace symmetrize {
namespace utils {

template <typename Weight, typename Profit> class KnapsackSolver {

public:
  virtual std::pair<Weight, std::vector<bool>>
  Solve(const std::vector<Weight> &weights, const std::vector<Profit> &profits,
        Weight capacity) const = 0;
};

template <typename Weight, typename Profit>
class GreedyApproximateKnapsackSolver : public KnapsackSolver<Weight, Profit> {

public:
  std::pair<Weight, std::vector<bool>> Solve(const std::vector<Weight> &weights,
                                             const std::vector<Profit> &profits,
                                             Weight capacity) const override {
    if (weights.size() != profits.size())
      throw std::invalid_argument("unequal amounts of weights and profits");
    std::vector<bool> res(weights.size(), false);

    std::multimap<double, std::pair<size_t, Weight>>
        objects; // profit / weight -> (index, weight)
    for (size_t i = 0; i < weights.size(); i++) {
      Weight w = weights[i];
      Profit p = profits[i];
      if (p < 0)
        continue;
      double pr;
      pr = w != 0 ? (double)p / (double)w
                  : std::numeric_limits<double>::infinity();
      objects.emplace(pr, std::pair<size_t, Weight>(i, w));
    }

    Weight used_capacity = 0;
    for (auto obj_it = objects.rbegin(); obj_it != objects.rend(); obj_it++) {
      auto obj = obj_it->second;
      size_t idx = obj.first;
      Weight w = obj.second;
      if (used_capacity + w > capacity)
        continue;
      used_capacity += w;
      res[idx] = true;
    }

    return {used_capacity, res};
  }
};

} // namespace utils
} // namespace symmetrize
