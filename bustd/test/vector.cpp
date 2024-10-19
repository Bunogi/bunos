#include <bustd/vector.hpp>
#include <libraries/libtest/libtest.hpp>

namespace {
auto basic_push_pop() -> test::Result {
  bu::Vector<int> foo;

  for (usize i = 0; i < 5; i++) {
    foo.push(i);
    LIBTEST_ASSERT_EQ(foo.len(), i + 1);
  }

  const auto current_capacity = foo.capacity();
  LIBTEST_ASSERT_EQ(foo.back(), 4);
  foo.pop();
  LIBTEST_ASSERT_EQ(foo.back(), 3);
  foo.pop();
  LIBTEST_ASSERT_EQ(foo.back(), 2);
  foo.pop();
  LIBTEST_ASSERT_EQ(foo.back(), 1);
  foo.pop();
  LIBTEST_ASSERT_EQ(foo.back(), 0);
  foo.pop();

  LIBTEST_ASSERT_EQ(current_capacity, foo.capacity());
  LIBTEST_SUCCEED();
}

auto pop_value() -> test::Result {
  bu::Vector<int> foo;

  foo.push(1);
  int i;
  foo.pop(i);
  LIBTEST_ASSERT_EQ(i, 1);

  LIBTEST_SUCCEED();
}

auto destructor_call_pop() -> test::Result {
  static int num_destroyed = 0;
  struct Destructable {
    ~Destructable() { num_destroyed++; }
  };

  {
    bu::Vector<Destructable> vec;
    Destructable d;
    vec.push(bu::move(d));

    vec.pop();
    LIBTEST_ASSERT_EQ(num_destroyed, 1);
    Destructable d1;
    vec.push(bu::move(d1));
    Destructable d2;
    vec.pop(d2);
    LIBTEST_ASSERT_EQ(num_destroyed, 2);
  }
  LIBTEST_ASSERT_EQ(num_destroyed, 5);

  LIBTEST_SUCCEED();
}

auto no_duplicate_constructor_call() -> test::Result {
  static int num_constructed = 0;
  struct Constructable {
    Constructable() { num_constructed++; }
  };

  {
    bu::Vector<Constructable> vec;
    Constructable c;
    vec.push(bu::move(c));

    vec.pop();
    LIBTEST_ASSERT_EQ(num_constructed, 1);
    Constructable c1;
    vec.push(bu::move(c1));
    Constructable c2;
    vec.pop(c2);
    LIBTEST_ASSERT_EQ(num_constructed, 3);
  }
  LIBTEST_ASSERT_EQ(num_constructed, 3);

  LIBTEST_SUCCEED();
}

auto allocation_strategy() -> test::Result {
  bu::Vector<int> v(10);
  LIBTEST_ASSERT_EQ(v.len(), 0);
  LIBTEST_ASSERT_EQ(v.capacity(), 10);

  bu::Vector<int> vec;
  LIBTEST_ASSERT_EQ(vec.capacity(), 0);
  vec.push(1);
  LIBTEST_ASSERT_EQ(vec.len(), 1);
  LIBTEST_ASSERT_EQ(vec.capacity(), 1);

  vec.push(2);
  LIBTEST_ASSERT_EQ(vec.len(), 2);
  LIBTEST_ASSERT_EQ(vec.capacity(), 2);

  for (int i = 0; i < 2; i++) {
    vec.push(i);
  }
  LIBTEST_ASSERT_EQ(vec.len(), 4);
  LIBTEST_ASSERT_EQ(vec.capacity(), 4);

  for (int i = 0; i < 4; i++) {
    vec.push(i);
  }
  LIBTEST_ASSERT_EQ(vec.len(), 8);
  LIBTEST_ASSERT_EQ(vec.capacity(), 8);
  vec.push(1);
  LIBTEST_ASSERT_EQ(vec.len(), 9);
  LIBTEST_ASSERT_EQ(vec.capacity(), 16);

  LIBTEST_SUCCEED();
}

auto preallocate() -> test::Result {
  const usize allocate = 10;
  bu::Vector<int> v(allocate);
  LIBTEST_ASSERT_EQ(v.len(), 0);
  LIBTEST_ASSERT_EQ(v.capacity(), allocate);

  for (usize i = 0; i < allocate; i++) {
    v.push(i);
    LIBTEST_ASSERT_EQ(v.len(), i + 1);
  }
  v.push(0);
  LIBTEST_ASSERT_EQ(v.len(), allocate + 1);
  LIBTEST_ASSERT_EQ(v.capacity(), 2 * allocate);

  LIBTEST_SUCCEED();
}

auto move() -> test::Result {
  static int num_constructed = 0;
  static int num_copied = 0;
  static int num_moved = 0;
  static int num_destroyed = 0;
  struct Copyable {
    Copyable() { num_constructed++; }
    Copyable(const Copyable &) { num_copied++; }
    Copyable(Copyable &&) { num_moved++; }
    ~Copyable() { num_destroyed++; }
  };

  bu::Vector<Copyable> v;
  constexpr u32 to_create = 0;
  for (u32 i = 0; i < to_create; i++) {
    v.push(Copyable());
  }
  num_constructed = num_copied = num_moved = num_destroyed = 0;

  bu::Vector<Copyable> copy = v;
  LIBTEST_ASSERT_EQ(num_constructed, 0);
  LIBTEST_ASSERT_EQ(num_copied, to_create);
  LIBTEST_ASSERT_EQ(num_moved, 0);
  LIBTEST_ASSERT_EQ(num_destroyed, 0);
  num_copied = 0;

  bu::Vector<Copyable> move = bu::move(v);

  LIBTEST_SUCCEED();
}

auto index() -> test::Result {
  bu::Vector<usize> v;
  v.push(0);
  v.push(1);
  v.push(2);

  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_EQ(v[i], i);
  }

  LIBTEST_SUCCEED();
}

auto fill() -> test::Result {
  bu::Vector<char> v;
  constexpr int num = 10;
  v.fill('0', num);
  LIBTEST_ASSERT_EQ(v.len(), num);
  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_EQ(v[i], '0');
  }

  v.fill('1', num);
  LIBTEST_ASSERT_EQ(v.len(), num);
  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_EQ(v[i], '1');
  }

  LIBTEST_SUCCEED();
}

auto resize_to_fit() -> test::Result {
  bu::Vector<int> v(300);
  LIBTEST_ASSERT_EQ(v.capacity(), 300);
  for (int i = 0; i < 150; i++) {
    v.push(i);
  }
  LIBTEST_ASSERT_EQ(v.len(), 150);
  LIBTEST_ASSERT_EQ(v.capacity(), 300);
  v.resize_to_fit();
  LIBTEST_ASSERT_EQ(v.len(), 150);
  LIBTEST_ASSERT_EQ(v.capacity(), 150);
  LIBTEST_SUCCEED();
}

auto clear() -> test::Result {
  bu::Vector<int> v(200);
  for (int i = 0; i < 200; i++) {
    v.push(i);
  }
  LIBTEST_ASSERT_EQ(v.len(), 200);
  LIBTEST_ASSERT_EQ(v.capacity(), 200);
  v.clear();
  LIBTEST_ASSERT_EQ(v.len(), 0);
  LIBTEST_ASSERT_EQ(v.capacity(), 200);
  LIBTEST_SUCCEED();
}

auto remove() -> test::Result {
  bu::Vector<usize> v;
  for (usize i = 0; i < 10; i++) {
    v.push(i);
  }

  v.remove(0);
  LIBTEST_ASSERT_EQ(v.len(), 9);
  LIBTEST_ASSERT_NE(v[0], 0);
  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_EQ(v[i], i + 1);
  }

  LIBTEST_SUCCEED();
}

auto remove_destructor_sanity() -> test::Result {
  static int num_destroyed = 0;
  // When constructing the vector, the destructor is called many times.
  static bool count_destructions = false;
  struct DestroyCounter {
    ~DestroyCounter() {
      if (count_destructions) {
        num_destroyed++;
      }
    }
  };
  {
    bu::Vector<DestroyCounter> v;
    for (int i = 0; i < 3; i++) {
      v.push(DestroyCounter());
    }

    count_destructions = true;
    v.remove(0);
    LIBTEST_ASSERT_EQ(num_destroyed, 1);
  }
  LIBTEST_ASSERT_EQ(num_destroyed, 3);
  LIBTEST_SUCCEED();
}

auto remove_if() -> test::Result {
  bu::Vector<usize> v;
  for (usize i = 0; i < 100; i++) {
    v.push(i);
  }

  const auto old_len = v.len();

  v.remove_if([](int v) { return v % 2 == 0; });

  LIBTEST_ASSERT_EQ(v.len(), old_len / 2);
  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_NE(v[i] % 2, 0);
  }
  LIBTEST_SUCCEED();
}

auto range_for() -> test::Result {
  bu::Vector<usize> v;
  for (usize i = 0; i < 50; i++) {
    v.push(i);
  }

  usize current = 0;
  for (auto &i : v) {
    LIBTEST_ASSERT_EQ(i, current++);
    i *= 2;
  }

  current = 0;
  for (const auto &i : v) {
    LIBTEST_ASSERT_EQ(i, 2 * current++);
  }

  current = 0;
  for (const auto i : v) {
    LIBTEST_ASSERT_EQ(i, 2 * current++);
  }

  current = 0;
  for (auto i : v) {
    LIBTEST_ASSERT_EQ(i, 2 * current++);
  }

  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_EQ(v[i], i * 2);
  }

  bu::Vector<usize> v2;
  for (auto &i : v2) {
    LIBTEST_ASSERT(false);
    (void)i;
  }
  LIBTEST_SUCCEED();
}

auto const_range_for() -> test::Result {
  bu::Vector<usize> v;
  for (usize i = 0; i < 50; i++) {
    v.push(i);
  }

  const auto v2 = v;

  usize current = 0;
  for (const auto &i : v2) {
    LIBTEST_ASSERT_EQ(i, current++);
  }

  current = 0;
  for (const auto i : v2) {
    LIBTEST_ASSERT_EQ(i, current++);
  }

  bu::Vector<usize> v3;
  for (const auto &i : v3) {
    LIBTEST_ASSERT(false);
    (void)i;
  }
  for (const auto i : v3) {
    LIBTEST_ASSERT(false);
    (void)i;
  }
  LIBTEST_SUCCEED();
}

} // namespace

auto main() -> int {
  RUN_TEST(allocation_strategy);
  RUN_TEST(basic_push_pop);
  RUN_TEST(clear);
  RUN_TEST(destructor_call_pop);
  RUN_TEST(fill);
  RUN_TEST(index);
  RUN_TEST(move);
  RUN_TEST(no_duplicate_constructor_call);
  RUN_TEST(pop_value);
  RUN_TEST(preallocate);
  RUN_TEST(range_for);
  RUN_TEST(const_range_for);
  RUN_TEST(remove);
  RUN_TEST(remove_destructor_sanity);
  RUN_TEST(remove_if);
  RUN_TEST(resize_to_fit);
  LIBTEST_CLEANUP();
}
