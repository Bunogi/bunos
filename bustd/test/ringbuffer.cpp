#include <bustd/assert.hpp>
#include <bustd/ringbuffer.hpp>
#include <bustd/string_view.hpp>
#include <libraries/libtest/libtest.hpp>
#include <string.h>

test::Result basic_read_take() {
  bu::RingBuffer buf;
  bu::StringView v = "Why did the chicken cross the road?";
  buf.write(v.data_u8(), v.len());
  LIBTEST_ASSERT_EQ(buf.len(), v.len());
  buf.write(v.data_u8(), v.len());
  LIBTEST_ASSERT_EQ(buf.len(), v.len() * 2);

  // TODO: switch to vector
  char *const otherbuf = new char[v.len() + 1];
  buf.take(reinterpret_cast<u8 *>(otherbuf), v.len());
  otherbuf[v.len()] = 0;
  LIBTEST_ASSERT_EQ(v, otherbuf);
  LIBTEST_ASSERT_EQ(buf.len(), v.len());

  delete[] otherbuf;

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

  printf("Dropping %lu values\n", len / 2);
  while (buf.len() > len / 2) {
    buf.drop(4);
  }

  while (!buf.is_full()) {
    buf.write(s.data_u8(), s.len());
  }

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
  LIBTEST_CLEANUP();
}
