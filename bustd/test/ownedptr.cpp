#include <libraries/libtest/libtest.hpp>

#include <bustd/assert.hpp>
#include <bustd/ownedptr.hpp>
#include <bustd/stddef.hpp>

namespace {
static u32 deleted_count, created_count;

static u32 pinged;

class Foo {
public:
  Foo() : bar(0) { created_count++; }
  Foo(int a) : bar(a) { created_count++; }
  auto ping() -> u32 { return ++pinged; }
  ~Foo() { deleted_count++; }
  u32 bar;
};

auto scope() -> test::Result {
  const auto created_before = created_count;
  const auto deleted_before = deleted_count;
  {
    bu::OwnedPtr<Foo> foo = bu::create_owned<Foo>();
    LIBTEST_ASSERT_EQ(created_count, created_before + 1);
    const auto pinged_before = pinged;
    LIBTEST_ASSERT_EQ(foo->ping(), pinged_before + 1);
  }
  LIBTEST_ASSERT_EQ(deleted_before + 1, deleted_count);
  LIBTEST_SUCCEED();
}

auto moved() -> test::Result {
  const auto deleted_before = deleted_count;
  {
    bu::OwnedPtr<Foo> foo1 = bu::create_owned<Foo>();
    const auto created_before = created_count;

    bu::OwnedPtr<Foo> foo2(bu::move(foo1));
    LIBTEST_ASSERT_EQ(created_before, created_count);
    LIBTEST_ASSERT_EQ(deleted_before, deleted_count);
  }
  // ASSERT_EQ(deleted_before + 1, deleted_count);

  LIBTEST_SUCCEED();
}

auto nullable() -> test::Result {
  bu::OwnedPtr<int> ptr(nullptr);
  LIBTEST_ASSERT_EQ(ptr.is_null(), true);
  LIBTEST_ASSERT_EQ(static_cast<bool>(ptr), false);

  ptr = bu::create_owned<int>(3);
  LIBTEST_ASSERT_EQ(ptr.is_null(), false);
  LIBTEST_ASSERT_EQ(static_cast<bool>(ptr), true);

  LIBTEST_SUCCEED();
}
} // namespace

auto main() -> int {
  RUN_TEST(scope);
  RUN_TEST(moved);
  RUN_TEST(nullable);
  LIBTEST_CLEANUP();
}
