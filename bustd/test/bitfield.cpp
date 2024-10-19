#include <bustd/assert.hpp>
#include <bustd/bitfield.hpp>
#include <libraries/libtest/libtest.hpp>

namespace {
template <usize N>
auto check_first_byte(bu::Bitfield<N> field) -> test::Result {
  LIBTEST_ASSERT(!field.replace(1, true));
  LIBTEST_ASSERT(field.replace(1, false));

  LIBTEST_ASSERT_EQ(field[3], false);
  LIBTEST_ASSERT_EQ(field.at(3), false);
  LIBTEST_SUCCEED();
}

auto basic() -> test::Result {
  bu::Bitfield<8> field;
  return check_first_byte(field);
}

auto multibyte() -> test::Result {
  bu::Bitfield<16> field;
  LIBTEST_ASSERT_EQ(field.size(), 16);

  LIBTEST_CHECK_OTHER(check_first_byte(field));

  LIBTEST_ASSERT(!field.replace(9, true));
  LIBTEST_ASSERT(field.replace(9, false));
  LIBTEST_SUCCEED();
}

auto small() -> test::Result {
  // less than one byte, should have one byte of storage and stuff
  bu::Bitfield<4> field;
  LIBTEST_CHECK_OTHER(check_first_byte(field));
  LIBTEST_SUCCEED();
}

auto small_overrun() -> test::Result {
  // One and a half bytes
  bu::Bitfield<12> field;
  LIBTEST_CHECK_OTHER(check_first_byte(field));
  LIBTEST_ASSERT(!field.replace(9, true));
  LIBTEST_ASSERT(field.replace(9, false));
  LIBTEST_SUCCEED();
}
} // namespace

auto main() -> int {
  RUN_TEST(basic);
  RUN_TEST(multibyte);
  RUN_TEST(small);
  RUN_TEST(small_overrun);
  LIBTEST_CLEANUP();
}
