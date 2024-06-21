#pragma once

#include <memory>
#include <stdexcept>

namespace symmetrize {

// ceil(ld(i))
template <typename T> T Log2(T i) {
  if (i <= 0)
    throw std::invalid_argument("log2 of " + std::to_string(i) +
                                " is undefined");
  i--;
  T ld = 0;
  for (; i > 0; i /= 2, ld++)
    ;
  return ld;
}

template <typename T> bool IsPow2(T i) { return i > 0 && ((i & (i - 1)) == 0); }

/**
 * Computes and holds all binomial coefficients i over j with i <= n and j <= n
 *
 * @tparam IntType type of binomial coefficient values
 */
template <class IntType> class BinomialCoefficients {

public:
  explicit BinomialCoefficients(size_t n);

  const IntType *operator[](size_t n) const;
  IntType at(size_t n, size_t k) const { return (*this)[n][k]; }

  size_t n() const { return n_; }

private:
  const size_t n_;
  std::unique_ptr<IntType[]> values_;
};

template <class IntType>
BinomialCoefficients<IntType>::BinomialCoefficients(size_t n)
    : n_(n), values_(std::make_unique<IntType[]>((n + 1) * (n + 2) / 2)) {
  values_[0] = 1; // 0 over 0 = 0
  IntType *drag = values_.get();
  IntType *curr = drag + 1;
  for (size_t i = 1; i <= n; i++) {
    *(curr++) = 1;
    for (size_t j = 1; j < i; j++) {
      IntType last = *drag;
      *(curr++) = last + *(++drag);
    }
    drag++;
    *(curr++) = 1;
  }
}

template <class IntType>
const IntType *BinomialCoefficients<IntType>::operator[](size_t n) const {
  return values_.get() + (n * (n + 1) / 2);
}

} // namespace symmetrize
