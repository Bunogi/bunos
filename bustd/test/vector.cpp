#include <bustd/vector.hpp>
#include <libraries/libtest/libtest.hpp>

test::Result basic_push_pop() {
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

test::Result pop_value() {
  bu::Vector<int> foo;

  foo.push(1);
  int i;
  foo.pop(i);
  LIBTEST_ASSERT_EQ(i, 1);

  LIBTEST_SUCCEED();
}

test::Result destructor_call_pop() {
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

test::Result no_duplicate_constructor_call() {
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

test::Result allocation_strategy() {
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

test::Result preallocate() {
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

test::Result move() {
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

test::Result index() {
  bu::Vector<usize> v;
  v.push(0);
  v.push(1);
  v.push(2);

  for (usize i = 0; i < v.len(); i++) {
    LIBTEST_ASSERT_EQ(v[i], i);
  }

  LIBTEST_SUCCEED();
}

test::Result fill() {
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

test::Result resize_to_fit() {
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

test::Result clear() {
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

test::Result remove() {
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

test::Result remove_if() {
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

test::Result range_for() {
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

int main() {
  RUN_TEST(basic_push_pop);
  RUN_TEST(pop_value);
  RUN_TEST(destructor_call_pop);
  RUN_TEST(no_duplicate_constructor_call);
  RUN_TEST(allocation_strategy);
  RUN_TEST(preallocate);
  RUN_TEST(index);
  RUN_TEST(fill);
  RUN_TEST(resize_to_fit);
  RUN_TEST(clear);
  RUN_TEST(remove);
  RUN_TEST(remove_if);
  RUN_TEST(range_for);
  LIBTEST_CLEANUP();
}
