#pragma once

#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <stdio.h>

namespace test {
typedef bool Result;
static int failures;
static int successes;
static const char *current_test_name = "";

void handle_result(Result res) {
  if (!res) {
    failures++;
  } else {
    successes++;
  }
}

} // namespace test

#define RUN_TEST(_test)                                                        \
  {                                                                            \
    ::test::current_test_name = #_test;                                        \
    const auto _res = _test();                                                 \
    ::test::handle_result(_res);                                               \
  }

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define LIBTEST_FAIL(_msg)                                                     \
  do {                                                                         \
    const char *const message = _msg " at " __FILE__ ":" TOSTRING(__LINE__);   \
    printf("\033[31m[FAIL]\033[m %s: %s\n", ::test::current_test_name,         \
           message);                                                           \
    return false;                                                              \
  } while (0)

#define LIBTEST_SUCCEED()                                                      \
  do {                                                                         \
    return true;                                                               \
  } while (0)

#define LIBTEST_CHECK_OTHER(v)                                                 \
  {                                                                            \
    ::test::Result _test_result = v;                                           \
    if (!_test_result) {                                                       \
      return false;                                                            \
    }                                                                          \
  }

#define LIBTEST_CLEANUP()                                                      \
  do {                                                                         \
    const auto total = ::test::failures + ::test::successes;                   \
    if (::test::failures != 0) {                                               \
      printf("%s: \033[31m%u/%u tests failed!\033[m\n", __FILE__,              \
             ::test::failures, total);                                         \
      return 1;                                                                \
    } else {                                                                   \
      printf("%s: \033[32mAll %u tests succeeded!\033[m\n", __FILE__, total);  \
      return 0;                                                                \
    }                                                                          \
  } while (0)

#define LIBTEST_ASSERT(_v)                                                     \
  do {                                                                         \
    if ((_v)) {                                                                \
    } else {                                                                   \
      LIBTEST_FAIL("Assertion failed: '" #_v "'");                             \
    }                                                                          \
  } while (0)

#define LIBTEST_ASSERT_EQ(_x, _y)                                              \
  do {                                                                         \
    const auto _lhs = _x;                                                      \
    const auto _rhs = _y;                                                      \
    if (_lhs == _rhs) {                                                        \
    } else {                                                                   \
      LIBTEST_FAIL("Assertion failed: '" #_x " == " #_y "'");                  \
    }                                                                          \
  } while (0)

#define LIBTEST_ASSERT_NE(_x, _y)                                              \
  do {                                                                         \
    const auto _lhs = _x;                                                      \
    const auto _rhs = _y;                                                      \
    if (_lhs != _rhs) {                                                        \
    } else {                                                                   \
      LIBTEST_FAIL("Assertion failed: '" #_x " != " #_y "'");                  \
    }                                                                          \
  } while (0)

#define __IN_LIBTEST__
