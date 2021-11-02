#include <bustd/function.hpp>
#include <bustd/optional.hpp>
#include <bustd/scopeguard.hpp>
#include <libtest/libtest.hpp>

using bu::create_none;
using bu::create_some;
using bu::Optional;

test::Result some_conversion() {
  const auto some = create_some<u8>(8);
  LIBTEST_ASSERT(static_cast<bool>(some));
  LIBTEST_SUCCEED();
}

test::Result none_conversion() {
  const auto none = create_none<u8>();
  LIBTEST_ASSERT(!static_cast<bool>(none));

  LIBTEST_SUCCEED();
}

test::Result with_data() {
  const auto some = create_some<u8>(128);
  LIBTEST_ASSERT(static_cast<bool>(some));
  LIBTEST_ASSERT_EQ(*some, 128);

  LIBTEST_SUCCEED();
}

test::Result moving_works() {
  Optional<u8> some = create_some<u8>(128);

  Optional<u8> other(bu::move(some));
  LIBTEST_ASSERT(!static_cast<bool>(some));
  LIBTEST_ASSERT(static_cast<bool>(other));
  LIBTEST_ASSERT_EQ(*other, 128);

  LIBTEST_SUCCEED();
}

static u32 destructor_calls;
struct DestructorCounter {
  ~DestructorCounter() { destructor_calls++; }
};

test::Result destructor_sanity() {
  u8 destructor_calls = 0;
  { const auto some = create_some<DestructorCounter>(); }
  ASSERT_EQ(destructor_calls, 1);

  {
    auto some = create_some<DestructorCounter>();
    auto other(bu::move(some));
  }
  ASSERT_EQ(destructor_calls, 2);
  LIBTEST_SUCCEED();
}

int main() {
  RUN_TEST(some_conversion);
  RUN_TEST(none_conversion);

  LIBTEST_CLEANUP();
}
