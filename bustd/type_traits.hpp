#pragma once
#include <bustd/stddef.hpp>

namespace bu {
template <class T, T v> struct IntegralConstant {
  static constexpr T value = v;
  using value_type = T;
  using type = IntegralConstant;
  constexpr operator value_type() const noexcept { return value; }
};

typedef IntegralConstant<bool, false> FalseType;
typedef IntegralConstant<bool, true> TrueType;

// FIXME: Find a good naming scheme for these
template <typename T> struct is_integral_type : FalseType {};
#define _MARK_INTEGRAL(_n)                                                     \
  template <> struct is_integral_type<_n> : TrueType {};
_MARK_INTEGRAL(u8)
_MARK_INTEGRAL(u16)
_MARK_INTEGRAL(u32)
_MARK_INTEGRAL(u64)
_MARK_INTEGRAL(i8)
_MARK_INTEGRAL(i16)
_MARK_INTEGRAL(i32)
_MARK_INTEGRAL(i64)
#ifdef __IN_KERNEL__
_MARK_INTEGRAL(int)
#endif
#undef _MARK_INTEGRAL

template <typename T, typename U> struct is_same : FalseType {};
template <typename T> struct is_same<T, T> : TrueType {};
} // namespace bu
