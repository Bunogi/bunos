#pragma once

#ifdef __IN_KERNEL__
#include <kernel/panic.hpp>
#define FAIL(message) KERNEL_PANIC(message)
#else
#include <stdio.h>
#include <stdlib.h>
// TODO: This should print a backtrace
#define FAIL(message)                                                          \
  do {                                                                         \
    printf("%s\n==>in %s()\n==>at %s:%u\n", message, __func__, __FILE__,       \
           __LINE__);                                                          \
    exit(1);                                                                   \
  } while (0)
#endif

#define ASSERT(_x)                                                             \
  do {                                                                         \
    if (_x) {                                                                  \
    } else {                                                                   \
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

#define UNREACHABLE()                                                          \
  do {                                                                         \
    FAIL("Reached unreachable code");                                          \
  } while (0)

#define TODO(_msg)                                                             \
  do {                                                                         \
    FAIL("Hit a work-in progress code path: '" _msg "'");                      \
  } while (0)
