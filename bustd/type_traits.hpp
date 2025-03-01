#pragma once
#include <bustd/stddef.hpp>

namespace bu {
template <class T, T v> struct IntegralConstant {
  static constexpr T value = v;
  using value_type = T;
  using type = IntegralConstant;
  constexpr operator value_type() const noexcept { return value; }
};

using FalseType = IntegralConstant<bool, false>;
using TrueType = IntegralConstant<bool, true>;

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
#undef _MARK_INTEGRAL

template <typename T, typename U> struct is_same : FalseType {};
template <typename T> struct is_same<T, T> : TrueType {};

template <bool B, class T = void> struct enable_if {};
template <class T> struct enable_if<true, T> {
  using type = T;
};

template <class T> struct _remove_const {
  using type = T;
};
template <class T> struct _remove_const<const T> {
  using type = T;
};
template <typename T> using remove_const = typename _remove_const<T>::type;

template <class T> struct _remove_volatile {
  using type = T;
};
template <class T> struct _remove_volatile<volatile T> {
  using type = T;
};
template <typename T>
using remove_volatile = typename _remove_volatile<T>::type;

template <typename T> using remove_cv = remove_const<remove_volatile<T>>;

template <class T> constexpr bool _is_pointer = false;
template <class T> constexpr bool _is_pointer<T *> = true;
template <class T> constexpr bool is_pointer = _is_pointer<remove_cv<T>>;

// All these can be found in https://gcc.gnu.org/onlinedocs/gcc/Type-Traits.html
template <typename T>
constexpr bool is_trivially_coyable = __is_trivially_copyable(T);

} // namespace bu
