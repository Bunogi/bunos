#include <bustd/algorithm.hpp>
#include <bustd/list.hpp>
#include <libtest/libtest.hpp>

using namespace bu;

auto find_if() -> test::Result {
  List<u8> data;
  for (int i = 0; i < 64; i++) {
    data.append_back(i);
  }

  const auto res =
      find_if(data.begin(), data.end(), [](const auto &i) { return i == 25; });
  LIBTEST_ASSERT_NE(res, data.end());
  LIBTEST_ASSERT_EQ(*res, 25);

  const auto res2 = find_if(data.begin(), data.end(),
                            [](const auto &i) { return i == 0xFF; });
  LIBTEST_ASSERT_EQ(res2, data.end());

  LIBTEST_SUCCEED();
}

auto main() -> int {
  RUN_TEST(find_if);

  LIBTEST_CLEANUP();
}
