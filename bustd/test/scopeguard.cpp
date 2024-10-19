#include <bustd/scopeguard.hpp>
#include <libtest/libtest.hpp>

namespace {
auto basic_locked() -> test::Result {
  bool did_execute = false;
  {
    bu::LockedScopeGuard guard([&]() { did_execute = true; });
    static_cast<void>(guard);
  }

  LIBTEST_ASSERT(did_execute);
  LIBTEST_SUCCEED();
}

auto basic() -> test::Result {
  bool did_execute = false;
  {
    bu::ScopeGuard guard([&]() { did_execute = true; });
    static_cast<void>(guard);
  }

  LIBTEST_ASSERT(did_execute);
  LIBTEST_SUCCEED();
}

auto disarmed() -> test::Result {
  bool did_execute = false;
  {
    bu::ScopeGuard guard([&]() { did_execute = true; });
    LIBTEST_ASSERT(guard.disarm());
    LIBTEST_ASSERT(!guard.armed());
  }

  LIBTEST_ASSERT(!did_execute);
  LIBTEST_SUCCEED();
}

auto rearmed() -> test::Result {
  bool did_execute = false;
  {
    bu::ScopeGuard guard([&]() { did_execute = true; });
    LIBTEST_ASSERT(guard.disarm());
    LIBTEST_ASSERT(!guard.armed());
    LIBTEST_ASSERT(!guard.rearm());
    LIBTEST_ASSERT(guard.armed());
  }

  LIBTEST_ASSERT(did_execute);
  LIBTEST_SUCCEED();
}
} // namespace

auto main() -> int {
  RUN_TEST(basic_locked);
  RUN_TEST(basic);
  RUN_TEST(disarmed);
  RUN_TEST(rearmed);
  LIBTEST_CLEANUP();
}
