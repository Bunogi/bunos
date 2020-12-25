#include <libraries/libtest/libtest.hpp>

#include <bustd/assert.hpp>
#include <bustd/ringbuffer.hpp>
#include <bustd/string_view.hpp>

test::Result basic_read_take() {
  bu::RingBuffer buf;
  bu::StringView v = "Why did the chicken cross the road?";
  buf.write(reinterpret_cast<const u8 *>(v.data()), v.len());
  ASSERT_EQ(buf.len(), v.len());
  buf.write(reinterpret_cast<const u8 *>(v.data()), v.len());
  ASSERT_EQ(buf.len(), v.len() * 2);

  // TODO: switch to vector
  char *otherbuf = new char[v.len() + 1];
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

  buf.write(reinterpret_cast<const u8 *>(s.data()), s.len());

  ASSERT_EQ(buf.take(otherbuf, 5), 3);
  ASSERT_EQ(buf.take(otherbuf, 5), 0);

  LIBTEST_SUCCEED();
}

test::Result with_wrapping() {
  bu::SizedRingBuffer<16> buf;
  bu::StringView s = "Hello, this is";
  buf.write(reinterpret_cast<const u8 *>(s.data()), s.len());

  u8 otherbuf[16];
  ASSERT_EQ(buf.take(otherbuf, 14), 14);

  ASSERT_EQ(buf.write(reinterpret_cast<const u8 *>(s.data()), s.len()),
            s.len());
  ASSERT_EQ(buf.read(otherbuf, s.len()), s.len());
  ASSERT(memcmp(s.data(), otherbuf, s.len()) == 0);

  LIBTEST_SUCCEED();
}

int main() {
  RUN_TEST(basic_read_take);
  RUN_TEST(read_empty);
  RUN_TEST(with_wrapping);
  LIBTEST_CLEANUP();
}
