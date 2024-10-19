#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>
#include <libtest/libtest.hpp>

using namespace bu::literals;

namespace {
auto comparison_basic() -> test::Result {
  constexpr auto s1 = "aaaa"sv;
  constexpr auto s2 = "aaaa"sv;
  LIBTEST_ASSERT_EQ(s1, s2);

  constexpr auto s3 = "bbbb"sv;
  LIBTEST_ASSERT_NE(s1, s3);

  constexpr auto s4 = "bbb"sv;
  LIBTEST_ASSERT_NE(s3, s4);

  LIBTEST_SUCCEED();
}

auto comparison_notnullterminated() -> test::Result {

  constexpr auto base = "aaaabbbb"sv;

  const auto s1 = base.substr(0, 4);
  const auto s2 = base.substr(0, 4);
  LIBTEST_ASSERT_EQ(s1, s2);

  const auto s3 = base.substr(5, base.len() - 1);
  LIBTEST_ASSERT_NE(s1, s3);

  const auto s4 = base.substr(2, 5);
  LIBTEST_ASSERT_NE(s3, s4);

  LIBTEST_SUCCEED();
}

auto substr() -> test::Result {
  constexpr auto base = "Hello this is a string"sv;
  const auto s1 = base.substr(0, 5);
  LIBTEST_ASSERT_EQ(s1, "Hello"sv);

  const auto s2 = base.substr(16, base.len());
  LIBTEST_ASSERT_EQ(s2, "string"sv);

  const auto s3 = base.substr(16, 16);
  LIBTEST_ASSERT_EQ(s3, ""sv);

  const auto s4 = base.substr(base.len(), base.len() + 1);
  LIBTEST_ASSERT_EQ(s4, ""sv);

  const auto s5 = base.substr(16, base.len() + 100);
  LIBTEST_ASSERT_EQ(s5, "string"sv);

  LIBTEST_SUCCEED();
}
} // namespace

auto main() -> int {
  RUN_TEST(comparison_basic);
  RUN_TEST(comparison_notnullterminated)
  RUN_TEST(substr);

  LIBTEST_CLEANUP();
}
