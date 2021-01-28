#include <bustd/rc.hpp>
#include <libtest/libtest.hpp>

test::Result basic_copy() {
  bu::Rc<usize> rc = bu::create_refcounted<usize>(0xdeadbeef);
  printf("%lu\n", rc.count());
  LIBTEST_ASSERT_EQ(*rc, 0xdeadbeef);
  printf("%lu\n", rc.count());
  LIBTEST_ASSERT_EQ(rc.count(), 1);

  {
    auto rc2 = rc;
    printf("%lu\n", rc2.count());
    LIBTEST_ASSERT_EQ(*rc2, 0xdeadbeef);
    LIBTEST_ASSERT_EQ(rc2.count(), 2);
    LIBTEST_ASSERT_EQ(rc.count(), 2);
  }
  LIBTEST_ASSERT_EQ(rc.count(), 1);

  auto other = bu::move(rc);
  printf("%lu\n", other.count());
  LIBTEST_ASSERT_EQ(other.count(), 1);
  LIBTEST_ASSERT_EQ(*other, 0xdeadbeef);

  LIBTEST_SUCCEED();
}

int main() {
  RUN_TEST(basic_copy);
  LIBTEST_CLEANUP();
}
