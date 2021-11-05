#include <bustd/function.hpp>
#include <bustd/optional.hpp>
#include <bustd/scopeguard.hpp>
#include <libtest/libtest.hpp>

using bu::create_none;
using bu::create_some;
using bu::Optional;

namespace {
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
  destructor_calls = 0;
  { const auto some = create_some<DestructorCounter>(); }
  LIBTEST_ASSERT_EQ(destructor_calls, 1);

  {
    auto some = create_some<DestructorCounter>();
    auto other(bu::move(some));
  }
  LIBTEST_ASSERT_EQ(destructor_calls, 2);
  LIBTEST_SUCCEED();
}

test::Result destructor_sanity_when_move_assigning() {
  destructor_calls = 0;
  {
    auto some = create_some<DestructorCounter>();
    auto other_some = create_some<DestructorCounter>();
    some = bu::move(other_some);
    LIBTEST_ASSERT_EQ(destructor_calls, 1);
  }
  LIBTEST_ASSERT_EQ(destructor_calls, 2);
  LIBTEST_SUCCEED();
}
} // namespace

int main() {
  RUN_TEST(some_conversion);
  RUN_TEST(none_conversion);
  RUN_TEST(with_data);
  RUN_TEST(moving_works);
  RUN_TEST(destructor_sanity);
  RUN_TEST(destructor_sanity_when_move_assigning);

  LIBTEST_CLEANUP();
}
