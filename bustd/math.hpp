#pragma once

#include <bustd/type_traits.hpp>

namespace bu {
template <typename T> inline constexpr auto min(const T &lhs, const T &rhs) {
  if (lhs < rhs) {
    return lhs;
  } else {
    return rhs;
  }
}

template <typename T> inline constexpr auto max(const T &lhs, const T &rhs) {
  if (lhs > rhs) {
    return lhs;
  } else {
    return rhs;
  }
}

template <typename T>
inline constexpr auto divide_ceil(const T &n, const T &div) {
  static_assert(is_integral_type<T>::value,
                "divide_ceil() only supports integer types");
  if (n % div != 0) {
    return n / div + 1;
  } else {
    return n / div;
  }
}

template <typename T> inline constexpr auto pow(const T &base, const T &pow) {
  static_assert(is_integral_type<T>::value,
                "Can only do powers of integer types for now");

  if (pow == 0) {
    return static_cast<T>(1);
  }
  T val = base;
  for (T i = 1; i < pow; i++) {
    val *= base;
  }
  return val;
}
} // namespace bu
