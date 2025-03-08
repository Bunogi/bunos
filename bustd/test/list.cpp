#include <bustd/list.hpp>
#include <libraries/libtest/libtest.hpp>

namespace {
auto basic_append() -> test::Result {
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

auto cleans_up() -> test::Result {
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

auto remove_if() -> test::Result {
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

auto range_for() -> test::Result {
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

auto transfer() -> test::Result {
  bu::List<usize> v;
  bu::List<usize> v2;

  v.append_back(1);
  v.append_back(2);
  v.append_back(3);
  v.append_back(4);

  // Middle
  {
    auto *const before = static_cast<void *>(&v.get(1));
    v2.take_back(v, 1);
    LIBTEST_ASSERT_EQ(v.len(), 3);

    LIBTEST_ASSERT_EQ(v.get(0), 1);
    LIBTEST_ASSERT_EQ(v.get(1), 3);
    LIBTEST_ASSERT_EQ(v.get(2), 4);

    LIBTEST_ASSERT_EQ(v2.len(), 1);
    auto *const after = static_cast<void *>(&v2.back());
    LIBTEST_ASSERT_EQ(before, after);
    LIBTEST_ASSERT_EQ(v2.get(0), 2);
  }

  // First
  {
    auto *const before = static_cast<void *>(&v.get(0));
    v2.take_back(v, 0);
    LIBTEST_ASSERT_EQ(v.len(), 2);

    LIBTEST_ASSERT_EQ(v.get(0), 3);
    LIBTEST_ASSERT_EQ(v.get(1), 4);

    LIBTEST_ASSERT_EQ(v2.len(), 2);
    auto *const after = static_cast<void *>(&v2.back());

    LIBTEST_ASSERT_EQ(before, after);
    LIBTEST_ASSERT_EQ(v2.get(0), 2);
    LIBTEST_ASSERT_EQ(v2.get(1), 1);
  }

  // last
  {
    auto *const before = static_cast<void *>(&v.get(v.len() - 1));
    v2.take_back(v, v.len() - 1);
    LIBTEST_ASSERT_EQ(v.len(), 1);

    LIBTEST_ASSERT_EQ(v.get(0), 3);

    LIBTEST_ASSERT_EQ(v2.len(), 3);
    auto *const after = static_cast<void *>(&v2.back());

    LIBTEST_ASSERT_EQ(before, after);
    LIBTEST_ASSERT_EQ(v2.get(0), 2);
    LIBTEST_ASSERT_EQ(v2.get(1), 1);
    LIBTEST_ASSERT_EQ(v2.get(2), 4);
  }

  // very last element
  {
    auto *const before = static_cast<void *>(&v.get(v.len() - 1));
    v2.take_back(v, 0);
    LIBTEST_ASSERT_EQ(v.len(), 0);
    LIBTEST_ASSERT_EQ(v2.len(), 4);
    auto *const after = static_cast<void *>(&v2.back());

    LIBTEST_ASSERT_EQ(before, after);
    LIBTEST_ASSERT_EQ(v2.get(0), 2);
    LIBTEST_ASSERT_EQ(v2.get(1), 1);
    LIBTEST_ASSERT_EQ(v2.get(2), 4);
    LIBTEST_ASSERT_EQ(v2.get(3), 3);
  }

  LIBTEST_SUCCEED();
}

auto looping_iterator_basic() -> test::Result {
  bu::List<int> l;
  {
    auto it = l.looping_iter();
    LIBTEST_ASSERT(!it);
  }
  constexpr auto items = 256;

  for (int i = 0; i < items; i++) {
    l.append_back(i);
  }

  auto it = l.looping_iter();

  for (int i = 0; i < items; i++) {
    LIBTEST_ASSERT(it);
    LIBTEST_ASSERT_EQ(*it, i);
    it++;
  }

  LIBTEST_ASSERT(it);
  LIBTEST_ASSERT_EQ(*it, 0);
  it.skip(items - 1);
  LIBTEST_ASSERT(it);
  LIBTEST_ASSERT_EQ(*it, items - 1);
  it.skip(items);
  LIBTEST_ASSERT(it);
  LIBTEST_ASSERT_EQ(*it, items - 1);
  it.skip(items * 2);
  LIBTEST_ASSERT(it);
  LIBTEST_ASSERT_EQ(*it, items - 1);

  it.skip(1);
  LIBTEST_ASSERT(it);
  LIBTEST_ASSERT_EQ(*it, 0);
  it.skip(2);
  LIBTEST_ASSERT(it);
  LIBTEST_ASSERT_EQ(*it, 2);

  LIBTEST_SUCCEED();
}

auto looping_iterator_remove() -> test::Result {
  auto get_list = [=]() {
    bu::List<int> l;

    for (int i = 0; i < 3; i++) {
      l.append_back(i);
    }
    return l;
  };

  {
    auto l = get_list();
    auto it = l.looping_iter();
    it = l.remove_loop(it);
    LIBTEST_ASSERT_EQ(l.len(), 2);
    LIBTEST_ASSERT(it);
    LIBTEST_ASSERT_EQ(*it, 1);
  }

  {
    auto l = get_list();
    auto it = l.looping_iter();
    it.skip(1);
    it = l.remove_loop(it);
    LIBTEST_ASSERT_EQ(l.len(), 2);
    LIBTEST_ASSERT(it);
    LIBTEST_ASSERT_EQ(*it, 2);
  }
  {
    auto l = get_list();
    auto it = l.looping_iter();
    it.skip(2);
    it = l.remove_loop(it);
    LIBTEST_ASSERT_EQ(l.len(), 2);
    LIBTEST_ASSERT(it);
    LIBTEST_ASSERT_EQ(*it, 0);
  }
  {
    auto l = get_list();
    auto it = l.looping_iter();
    it = l.remove_loop(it);
    it = l.remove_loop(it);
    it = l.remove_loop(it);
    LIBTEST_ASSERT(!it);
    LIBTEST_ASSERT(l.empty());
  }

  LIBTEST_SUCCEED();
}

} // namespace

auto main() -> int {
  RUN_TEST(basic_append);
  RUN_TEST(cleans_up);
  RUN_TEST(remove_if);
  RUN_TEST(range_for);
  RUN_TEST(transfer);
  RUN_TEST(looping_iterator_basic);
  RUN_TEST(looping_iterator_remove);
  LIBTEST_CLEANUP();
}
