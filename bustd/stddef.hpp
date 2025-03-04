#pragma once

#include <stddef.h>
#include <stdint.h>

namespace bu {
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = size_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
#ifdef __IS_X86__
using isize = int32_t;
#endif

using nullptr_t = decltype(nullptr);

template <class T> struct remove_reference {
  using type = T;
};
template <class T> struct remove_reference<T &> {
  using type = T;
};
template <class T> struct remove_reference<T &&> {
  using type = T;
};
template <typename T> auto move(T &v) noexcept -> T && {
  return static_cast<typename remove_reference<T>::type &&>(v);
}
template <class T> auto forward(T &v) noexcept -> T && {
  return static_cast<T &&>(v);
}
template <class T> auto forward(T &&v) noexcept -> T && {
  return static_cast<T &&>(v);
}

constexpr usize KiB = 1024;
constexpr usize MiB = KiB * 1024;

} // namespace bu

using bu::u16;
using bu::u32;
using bu::u64;
using bu::u8;
using bu::usize;

using bu::i16;
using bu::i32;
using bu::i64;
using bu::i8;
using bu::isize;
