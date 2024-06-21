#pragma once

#include <vector>

namespace symmetrize {
namespace utils {

template <typename T, typename It> class Array {

public:
  Array(It begin, It end) : begin_(begin), end_(end){};

  template <typename Iterable>
  Array(const Iterable &iterable) : Array(iterable.begin(), iterable.end()) {}

  template <typename Iterable>
  Array(Iterable &iterable) : Array(iterable.begin(), iterable.end()) {}

  size_t size() const { return end_ - begin_; }
  bool empty() const { return size() == 0; }

  It begin() const { return begin_; }
  It end() const { return end_; }

  const T &operator[](size_t i) const { return begin_[i]; }
  template <typename R, typename It1, typename It2>
  friend bool operator==(const Array<R, It1> &a, const Array<R, It2> &b);

  Array first(size_t n) const {
    n = std::min(n, size());
    return {begin_, begin_ + n};
  }
  Array last(size_t n) const {
    n = std::min(n, size());
    return {end_ - n, end_};
  }
  Array subarray(size_t begin = 0, size_t end = SIZE_MAX) const {
    if (size() == 0)
      return {end_, end_};
    begin = std::min(begin, size() - 1);
    end = std::max(begin, std::min(end, size()));
    return {begin_ + begin, begin_ + end};
  }

  bool all_equal(const T &to) const {
    for (auto it = begin(); it != end(); it++) {
      if (*it != to)
        return false;
    }
    return true;
  }

  bool all_equal() const {
    if (begin_ == end_)
      return true;
    return all_equal(*begin_);
  }

  template <typename It2> bool starts_with(const Array<T, It2> &other) const {
    if (other.size() > size())
      return false;
    auto it = begin();
    auto o_it = other.begin();
    for (; o_it != other.end(); it++, o_it++) {
      if (*it != *o_it)
        return false;
    }
    return true;
  }

private:
  It begin_;
  It end_;
};

template <typename T, typename It1, typename It2>
bool operator==(const Array<T, It1> &a, const Array<T, It2> &b) {
  if (a.size() != b.size())
    return false;
  auto a_it = a.begin();
  auto b_it = b.begin();
  while (a_it != a.end()) {
    if (*a_it != *b_it)
      return false;
    a_it++;
    b_it++;
  }
  return true;
}

} // namespace utils
} // namespace symmetrize
