#include <bustd/list.hpp>
#include <libraries/libtest/libtest.hpp>

test::Result basic_append() {
  bu::List<int> l;
  l.append_back(1);
  LIBTEST_ASSERT_EQ(l.back(), 1);
  LIBTEST_ASSERT_EQ(l.front(), 1);
  LIBTEST_ASSERT_EQ(l.len(), 1);

  l.append_front(2);
  LIBTEST_ASSERT_EQ(l.front(), 2);
  LIBTEST_ASSERT_EQ(l.back(), 1);
  LIBTEST_ASSERT_EQ(l.len(), 2);

  LIBTEST_SUCCEED();
}

test::Result cleans_up() {
  static int num_deleted;
  struct Deletable {
    Deletable() {}
    Deletable(Deletable &&other) { other.m_valid = false; }
    ~Deletable() {
      if (m_valid) {
        num_deleted++;
      }
    }

  private:
    bool m_valid{true};
  };

  bu::List<Deletable> l;
  for (int i = 0; i < 3; i++) {
    l.emplace_back(Deletable());
  }

  LIBTEST_ASSERT_EQ(num_deleted, 0);
  l.clear();
  LIBTEST_ASSERT_EQ(num_deleted, 3);

  num_deleted = 0;
  {
    bu::List<Deletable> l2;
    for (int i = 0; i < 3; i++) {
      l2.emplace_back(Deletable());
    }
  }
  LIBTEST_ASSERT_EQ(num_deleted, 3);

  num_deleted = 0;
  for (int i = 0; i < 3; i++) {
    l.emplace_back(Deletable());
  }
  l.remove(1);
  LIBTEST_ASSERT_EQ(num_deleted, 1);
  LIBTEST_ASSERT_EQ(l.len(), 2);
  LIBTEST_SUCCEED();
}

test::Result remove_if() {
  bu::List<usize> v;
  for (usize i = 0; i < 100; i++) {
    v.append_front(i);
  }

  v.remove_if([](int v) { return v % 2 == 0; });

  LIBTEST_ASSERT_EQ(v.len(), 50);
  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_NE(v.get(i) % 2, 0);
  }
  LIBTEST_SUCCEED();
}

test::Result range_for() {
  bu::List<usize> v;
  for (usize i = 0; i < 50; i++) {
    v.append_back(i);
  }

  // Mutable iterator
  usize current = 0;
  for (auto &i : v) {
    LIBTEST_ASSERT_EQ(i, current++);
    i *= 2;
  }

  current = 0;
  for (const auto &i : v) {
    LIBTEST_ASSERT_EQ(i, 2 * current++);
  }

  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_EQ(v.get(i), i * 2);
  }

  bu::List<usize> v2;
  for (auto &i : v2) {
    LIBTEST_ASSERT(false);
    (void)i;
  }

  LIBTEST_SUCCEED();
}

int main() {
  RUN_TEST(basic_append);
  RUN_TEST(cleans_up);
  RUN_TEST(remove_if);
  RUN_TEST(range_for);
  LIBTEST_CLEANUP();
}
