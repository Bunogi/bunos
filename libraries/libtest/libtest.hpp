#pragma once

#include <bustd/stddef.hpp>
#include <bustd/string_view.hpp>
#include <stdio.h>

namespace test {
typedef bool Result;
static int failures;
static int successes;
static const char *message;

void handle_result(const char *name, Result res) {
  if (!res) {
    printf("\033[31m[FAIL]\033[m %s: %s\n", name, message);
    failures++;
  } else {
    printf("\033[32m[PASS]\033[m %s\n", name);
    successes++;
  }
}

} // namespace test

#define RUN_TEST(_test)                                                        \
  {                                                                            \
    const auto _res = _test();                                                 \
    ::test::handle_result(#_test, _res);                                       \
  }

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define LIBTEST_FAIL(_msg)                                                     \
  do {                                                                         \
    ::test::message = _msg " at " __FILE__ ":" TOSTRING(__LINE__);             \
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
      printf("\033[31m%u/%u tests failed!\033[m\n", ::test::failures, total);  \
      return 1;                                                                \
    } else {                                                                   \
      printf("\033[32mAll %u tests succeeded!\033[m\n", total);                \
      return 0;                                                                \
    }                                                                          \
  } while (0)

#define __IN_LIBTEST__
