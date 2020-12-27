#include <libraries/libtest/libtest.hpp>

#include <bustd/assert.hpp>
#include <bustd/ringbuffer.hpp>
#include <bustd/string_view.hpp>
#include <string.h>

test::Result basic_read_take() {
  bu::RingBuffer buf;
  bu::StringView v = "Why did the chicken cross the road?";
  buf.write(v.data_u8(), v.len());
  ASSERT_EQ(buf.len(), v.len());
  buf.write(v.data_u8(), v.len());
  ASSERT_EQ(buf.len(), v.len() * 2);

  // TODO: switch to vector
  char *const otherbuf = new char[v.len() + 1];
  buf.take(reinterpret_cast<u8 *>(otherbuf), v.len());
  ASSERT_EQ(v, otherbuf);
  ASSERT_EQ(buf.len(), v.len());

  delete[] otherbuf;

  LIBTEST_SUCCEED();
}

test::Result read_empty() {
  bu::RingBuffer buf;
  bu::StringView s = "Foo";
  // TODO: Switch to vector
  u8 otherbuf[5];

  buf.write(s.data_u8(), s.len());

  ASSERT_EQ(buf.take(otherbuf, 5), 3);
  ASSERT_EQ(buf.take(otherbuf, 5), 0);

  LIBTEST_SUCCEED();
}

test::Result with_wrapping() {
  bu::SizedRingBuffer<16> buf;
  bu::StringView s = "Hello, this is";
  printf("s.len(): %lu\n", s.len());
  buf.write(s.data_u8(), s.len());

  u8 otherbuf[16];
  ASSERT_EQ(buf.take(otherbuf, 16), 14);

  ASSERT_EQ(buf.write(s.data_u8(), s.len()), s.len());
  ASSERT_EQ(buf.read(otherbuf, s.len()), s.len());
  for (int i = 0; i < 16; i++) {
    printf("Otherbuf[%d] = %c(%u)\n", i, otherbuf[i], otherbuf[i]);
  }
  ASSERT(memcmp(s.data_u8(), otherbuf, s.len()) == 0);

  LIBTEST_SUCCEED();
}

test::Result overrun_buffer() {
  constexpr usize size = 10;
  bu::SizedRingBuffer<size> buf;
  bu::StringView s = "Hello this is longer than 10 chars";
  ASSERT_EQ(buf.write(s.data_u8(), s.len()), size);

  LIBTEST_SUCCEED();
}

test::Result write_overrun_and_wrap() {
  constexpr usize size = 20;
  bu::SizedRingBuffer<size> buf;
  bu::StringView s = "1111111111";
  ASSERT_EQ(buf.write(s.data_u8(), s.len()), s.len());

  ASSERT_EQ(buf.drop(10), 10);

  bu::StringView s2 = "Hello this is longer than 10 chars";
  ASSERT_EQ(buf.write(s2.data_u8(), s2.len()), size);

  LIBTEST_SUCCEED();
}

test::Result many_reads_and_writes() {
  bu::SizedRingBuffer<20> buf;
  bu::StringView s = "This should work :^)";
  for (int i = 0; i <= 20; i++) {
    printf("i: %u ", i);
    ASSERT_EQ(buf.write(s.data_u8(), i), i);
    ASSERT_EQ(buf.drop(i), i);
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
  LIBTEST_CLEANUP();
}
