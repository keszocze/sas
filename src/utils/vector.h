#pragma once

#include <vector>

namespace symmetrize {
namespace utils {

template <typename T>
std::pair<std::vector<T>, std::vector<T>> Split(const std::vector<T> &v) {
  if (v.size() % 2 != 0)
    throw std::invalid_argument("vector size has to be even");
  size_t h = v.size() / 2;
  std::vector<T> a;
  a.reserve(h);
  std::vector<T> b;
  b.reserve(h);
  for (size_t i = 0; i < h; i++) {
    a.emplace_back(v[i]);
    b.emplace_back(v[i + h]);
  }
  return {a, b};
}

template <typename T>
std::vector<T> Select(const std::vector<T> &t, const std::vector<T> &f,
                      const std::vector<bool> &s) {
  if (t.size() != f.size() || f.size() != s.size()) {
    throw std::invalid_argument("vectors have to have same length");
  }
  std::vector<T> res;
  res.reserve(t.size());
  auto si = s.begin();
  for (auto ti = t.begin(), fi = f.begin(); ti != t.end(); ti++, fi++, si++) {
    res.emplace_back(*si ? *ti : *fi);
  }
  return res;
}

} // namespace utils
} // namespace symmetrize
