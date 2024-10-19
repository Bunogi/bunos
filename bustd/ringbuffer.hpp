#pragma once

#include <bustd/assert.hpp>
#include <bustd/macros.hpp>
#include <bustd/math.hpp>
#include <bustd/stddef.hpp>
#include <bustd/stringview.hpp>
#include <string.h>
namespace bu {
// TODO: This does not have to be a template class when we get a memory
// allocator
template <usize N = 1024> class SizedRingBuffer {
  BU_NOCOPY(SizedRingBuffer)

public:
  explicit SizedRingBuffer() = default;
  // TODO: When converted to a non-template class, implement these
  SizedRingBuffer(SizedRingBuffer &&other) = delete;
  auto operator=(SizedRingBuffer &&other) -> SizedRingBuffer = delete;
  // TODO: Maybe these should be a generic interface?
  [[nodiscard]] auto len() const -> usize { return m_current_size; }
  [[nodiscard]] auto max_len() const -> usize { return N; }
  [[nodiscard]] auto is_full() const -> bool { return m_current_size == N; }
  [[nodiscard]] auto is_empty() const -> bool { return m_current_size == 0; }
  auto write(const u8 *buffer, const usize length) -> usize {
    const auto to_write = bu::min(length, remaining_space());

    const auto append_offset = m_buffer_start + m_current_size;
    if (append_offset + to_write <= N) {
      // Can write directly
      memcpy(m_buffer + append_offset, buffer, to_write);
    } else if (append_offset < N) {
      // Have to divide it into two writes
      const auto margin_to_end = N - append_offset;
      memcpy(m_buffer + append_offset, buffer, margin_to_end);
      const auto remaining = to_write - margin_to_end;
      memcpy(m_buffer, buffer + margin_to_end, remaining);
    } else {
      // The whole write has to wrap
      const auto write_offset = append_offset - N;
      memcpy(m_buffer + write_offset, buffer, to_write);
    }
    m_current_size += to_write;
    return to_write;
  }

  auto read(u8 *buffer, const usize length) const -> usize {
    const auto to_read = bu::min(length, m_current_size);
    if (to_read == 0) {
      return 0;
    }

    const auto margin = N - m_buffer_start;
    if (margin > to_read) {
      memcpy(buffer, m_buffer + m_buffer_start, to_read);
    } else {
      const auto remaining = to_read - margin;
      memcpy(buffer, m_buffer + m_buffer_start, margin);
      memcpy(buffer + margin, m_buffer, remaining);
    }
    return to_read;
  }

  auto take(u8 *buffer, const usize upto) -> usize {
    const auto bytes_read = read(buffer, upto);
    drop(bytes_read);
    return bytes_read;
  }

  void push(const u8 byte) { write(&byte, 1); }

  auto pop() -> u8 {
    u8 retval;
    ASSERT_EQ(take(&retval, 1), 1);
    return retval;
  }

  [[nodiscard]] auto head() const -> u8 {
    u8 retval;
    ASSERT_EQ(read(&retval, 1), 1);
    return retval;
  }

  [[nodiscard]] auto remaining_space() const -> usize {
    return N - m_current_size;
  }
  // This was needed for the serial driver
  [[nodiscard]] auto vol_remaining_space() const volatile -> usize {
    return N - m_current_size;
  }

  // Restrictions: Calling write() invalidates these pointers, and no guarantees
  // are made about the state of these afterwards.
  auto read_nocopy(StringView &first_part, StringView &second_part) -> bool {
    if (is_empty()) {
      return false;
    }
    const auto margin = N - m_buffer_start;
    const auto buffer_casted = reinterpret_cast<const char *>(m_buffer);
    first_part =
        StringView(buffer_casted + m_buffer_start, min(margin, m_current_size));
    if (m_current_size > margin) {
      const auto remaining = m_current_size - margin;
      second_part = StringView(buffer_casted, remaining);
    } else {
      second_part = StringView(nullptr);
    }
    return true;
  }

  auto drop(const usize n) -> usize {
    const auto to_drop = bu::min(n, m_current_size);
    m_current_size -= to_drop;
    m_buffer_start += to_drop;
    if (m_buffer_start >= N) {
      m_buffer_start -= N;
    }
    return to_drop;
  }

  constexpr auto capacity() -> usize { return N; }

  void clear() { drop(m_current_size); }

private:
  usize m_buffer_start{0};
  usize m_current_size{0};
  // TODO: This should be dynamically allocated in the future.
  u8 m_buffer[N]{};
};

using RingBuffer = SizedRingBuffer<>;

} // namespace bu
