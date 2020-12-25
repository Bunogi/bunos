#pragma once

#ifdef __IN_LIBTEST__
#define FAIL(message) LIBTEST_FAIL(message)
#elif __IN_KERNEL__
#include <kernel/panic.hpp>
#define FAIL(message) KERNEL_PANIC(message)
#else
#error Asserts without libtest and not in kernel are not implemented
#endif

#define ASSERT(_x)                                                             \
  do {                                                                         \
    if ((_x) != true) {                                                        \
      FAIL("Assertion failed: '" #_x "'");                                     \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(_x, _y)                                                      \
  do {                                                                         \
    const auto _lhs = _x;                                                      \
    const auto _rhs = _y;                                                      \
    if (_lhs != _rhs) {                                                        \
      FAIL("Assertion failed: '" #_x "' == '" #_y "'");                        \
    }                                                                          \
  } while (0)

#define ASSERT_NE(_x, _y)                                                      \
  do {                                                                         \
    const auto _lhs = _x;                                                      \
    const auto _rhs = _y;                                                      \
    if (_lhs == _rhs) {                                                        \
      FAIL("Assertion failed: '" #_x "' != '" #_y "'");                        \
    }                                                                          \
  } while (0)

#define ASSERT_NOT_REACHED()                                                   \
  do {                                                                         \
    FAIL("Reached unreachable code");                                          \
  } while (0)
