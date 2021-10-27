#include <bustd/assert.hpp>
#include <bustd/ringbuffer.hpp>
#include <bustd/stringview.hpp>
#include <bustd/vector.hpp>
#include <libraries/libtest/libtest.hpp>
#include <string.h>

using namespace bu::literals;

test::Result basic_read_take() {
  bu::RingBuffer buf;
  auto v = "Why did the chicken cross the road?"sv;
  buf.write(v.data_u8(), v.len());
  LIBTEST_ASSERT_EQ(buf.len(), v.len());
  buf.write(v.data_u8(), v.len());
  LIBTEST_ASSERT_EQ(buf.len(), v.len() * 2);

  // TODO: switch to Array-type or something
  bu::Vector<char> otherbuf(v.len() + 1);
  otherbuf.fill(0, otherbuf.capacity());
  buf.take(reinterpret_cast<u8 *>(otherbuf.data()), v.len());
  otherbuf[v.len()] = 0;

  LIBTEST_ASSERT_EQ(strcmp(v.data(), otherbuf.data()), 0);
  LIBTEST_ASSERT_EQ(buf.len(), v.len());
  LIBTEST_SUCCEED();
}

test::Result read_empty() {
  bu::RingBuffer buf;
  bu::StringView s = "Foo";
  // TODO: Switch to vector
  u8 otherbuf[5];

  buf.write(s.data_u8(), s.len());

  LIBTEST_ASSERT_EQ(buf.take(otherbuf, 5), 3);
  LIBTEST_ASSERT_EQ(buf.take(otherbuf, 5), 0);

  LIBTEST_SUCCEED();
}

test::Result with_wrapping() {
  bu::SizedRingBuffer<16> buf;
  bu::StringView s = "Hello, this is";
  buf.write(s.data_u8(), s.len());

  u8 otherbuf[16];
  LIBTEST_ASSERT_EQ(buf.take(otherbuf, 16), 14);

  LIBTEST_ASSERT_EQ(buf.write(s.data_u8(), s.len()), s.len());
  LIBTEST_ASSERT_EQ(buf.read(otherbuf, s.len()), s.len());
  LIBTEST_ASSERT(memcmp(s.data_u8(), otherbuf, s.len()) == 0);

  LIBTEST_SUCCEED();
}

test::Result overrun_buffer() {
  constexpr usize size = 10;
  bu::SizedRingBuffer<size> buf;
  bu::StringView s = "Hello this is longer than 10 chars";
  LIBTEST_ASSERT_EQ(buf.write(s.data_u8(), s.len()), size);

  LIBTEST_SUCCEED();
}

test::Result write_overrun_and_wrap() {
  constexpr usize size = 20;
  bu::SizedRingBuffer<size> buf;
  bu::StringView s = "1111111111";
  LIBTEST_ASSERT_EQ(buf.write(s.data_u8(), s.len()), s.len());

  LIBTEST_ASSERT_EQ(buf.drop(10), 10);

  bu::StringView s2 = "Hello this is longer than 10 chars";
  LIBTEST_ASSERT_EQ(buf.write(s2.data_u8(), s2.len()), size);

  LIBTEST_SUCCEED();
}

test::Result many_reads_and_writes() {
  bu::SizedRingBuffer<20> buf;
  bu::StringView s = "This should work :^)";
  for (u32 i = 0; i <= 20; i++) {
    LIBTEST_ASSERT_EQ(buf.write(s.data_u8(), i), i);
    LIBTEST_ASSERT_EQ(buf.drop(i), i);
  }
  LIBTEST_SUCCEED();
}

// This test detects stack smashing, which I have had a ton of issues with.
test::Result write_until_full() {
  constexpr usize len = 128;
  bu::SizedRingBuffer<len> buf;
  bu::StringView s = "This is a long line :^)";
  while (!buf.is_full()) {
    buf.write(s.data_u8(), s.len());
  }

  while (buf.len() > len / 2) {
    buf.drop(4);
  }

  while (!buf.is_full()) {
    buf.write(s.data_u8(), s.len());
  }

  LIBTEST_SUCCEED();
}

test::Result push_pop() {
  bu::SizedRingBuffer<10> buf;
  for (int i = 0; i < 10; i++) {
    buf.push(i);
  }

  LIBTEST_ASSERT_EQ(buf.len(), 10);
  LIBTEST_ASSERT_EQ(buf.head(), 9);

  for (int i = 10; i >= 0; i++) {
    LIBTEST_ASSERT_EQ(buf.pop(), i);
  }
  LIBTEST_ASSERT(buf.is_empty());

  LIBTEST_SUCCEED();
}

test::Result read_nocopy() {
  bu::SizedRingBuffer<10> buf;

  constexpr auto half_capacity = buf.capacity() / 2;
  // Split the buffer in half with wrapping
  for (usize i = 0; i < half_capacity; i++) {
    buf.push('A');
  }
  LIBTEST_ASSERT_EQ(buf.drop(buf.len()), half_capacity);
  for (usize i = 0; i < half_capacity; i++) {
    buf.push('A');
  }
  for (usize i = buf.len(); i < buf.capacity(); i++) {
    buf.push('B');
  }
  LIBTEST_ASSERT_EQ(buf.len(), buf.capacity());

  bu::StringView lhs, rhs;
  LIBTEST_ASSERT(buf.read_nocopy(lhs, rhs));
  LIBTEST_ASSERT(lhs);
  LIBTEST_ASSERT(rhs);
  char left[half_capacity];
  char right[half_capacity];
  memset(left, 'A', half_capacity);
  memset(right, 'B', half_capacity);

  LIBTEST_ASSERT_EQ(lhs, bu::StringView(left, half_capacity));
  LIBTEST_ASSERT_EQ(rhs, bu::StringView(right, half_capacity));
  LIBTEST_SUCCEED();
}

int main() {
  RUN_TEST(basic_read_take);
  RUN_TEST(read_empty);
  RUN_TEST(with_wrapping);
  RUN_TEST(overrun_buffer);
  RUN_TEST(write_overrun_and_wrap);
  RUN_TEST(many_reads_and_writes);
  RUN_TEST(write_until_full);
  RUN_TEST(read_nocopy);
  LIBTEST_CLEANUP();
}
