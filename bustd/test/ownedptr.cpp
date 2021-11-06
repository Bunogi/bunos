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
  u32 ping() { return ++pinged; }
  ~Foo() { deleted_count++; }
  u32 bar;
};

test::Result scope() {
  const auto created_before = created_count;
  const auto deleted_before = deleted_count;
  {
    bu::OwnedPtr<Foo> foo = bu::create_owned<Foo>();
    ASSERT_EQ(created_count, created_before + 1);
    const auto pinged_before = pinged;
    ASSERT_EQ(foo->ping(), pinged_before + 1);
  }
  ASSERT_EQ(deleted_before + 1, deleted_count);
  LIBTEST_SUCCEED();
}

test::Result moved() {
  const auto deleted_before = deleted_count;
  {
    bu::OwnedPtr<Foo> foo1 = bu::create_owned<Foo>();
    const auto created_before = created_count;

    bu::OwnedPtr<Foo> foo2(bu::move(foo1));
    ASSERT_EQ(created_before, created_count);
    ASSERT_EQ(deleted_before, deleted_count);
  }
  ASSERT_EQ(deleted_before + 1, deleted_count);

  LIBTEST_SUCCEED();
}

test::Result nullable() {
  bu::OwnedPtr<int> ptr(nullptr);
  ASSERT_EQ(ptr.is_null(), true);
  ASSERT_EQ(static_cast<bool>(ptr), false);

  ptr = bu::create_owned<int>(3);
  ASSERT_EQ(ptr.is_null(), false);
  ASSERT_EQ(static_cast<bool>(ptr), true);

  LIBTEST_SUCCEED();
}
} // namespace

int main() {
  RUN_TEST(scope);
  RUN_TEST(moved);
  RUN_TEST(nullable);
  LIBTEST_CLEANUP();
}
