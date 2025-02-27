#pragma once

#include <stddef.h>
#include <stdint.h>

namespace bu {
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
#ifdef __IS_X86__
typedef int32_t isize;
#endif

typedef decltype(nullptr) nullptr_t;

template <class T> struct remove_reference {
  typedef T type;
};
template <class T> struct remove_reference<T &> {
  typedef T type;
};
template <class T> struct remove_reference<T &&> {
  typedef T type;
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
