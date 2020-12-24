#pragma once

#include <bustd/assert.hpp>
#include <bustd/macros.hpp>
#include <bustd/math.hpp>
#include <bustd/stddef.hpp>
#include <bustd/string_view.hpp>

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
  SizedRingBuffer operator=(SizedRingBuffer &&other) = delete;
  // TODO: Maybe these should be a generic interface?
  usize len() const { return m_current_size; }
  usize max_len() const { return N; }
  bool is_full() const { return m_current_size == N; }
  bool is_empty() const { return m_current_size == 0; }
  usize write(const u8 *buffer, const usize length) {
    // TODO: Handle gracefully when we don't have enough space
    ASSERT(N - m_current_size >= length);

    const auto margin = N - m_buffer_start;
    if (length < margin) {
      // Can write directly
      memcpy(m_buffer + m_buffer_start + m_current_size, buffer, length);
    } else {
      // Need to wrap
      memcpy(m_buffer + m_buffer_start + m_current_size, buffer, margin);
      const auto remaining = length - margin;
      memcpy(m_buffer, buffer + margin, remaining);
    }
    m_current_size += length;
    return length;
  }
  usize read(u8 *buffer, const usize length) const {
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
  usize take(u8 *buffer, const usize upto) {
    const auto bytes_read = read(buffer, upto);
    m_current_size -= bytes_read;
    m_buffer_start += bytes_read;

    if (m_buffer_start >= N) {
      m_buffer_start -= N;
    }
    return bytes_read;
  }

  // Restrictions: Calling write() invalidates these pointers, and no guarantees
  // are made about the state of these afterwards.
  bool take_nocopy(StringView &first_part, StringView &second_part) {
    if (is_empty()) {
      return false;
    }
    const auto margin = N - m_buffer_start;
    first_part =
        StringView(m_buffer + m_buffer_start, min(margin, m_current_size));
    if (m_current_size > margin) {
      const auto remaining = m_current_size - margin;
      second_part = StringView(m_buffer, remaining);
    } else {
      second_part = StringView(nullptr);
    }
  }

private:
  usize m_buffer_start{0};
  usize m_current_size{0};
  // TODO: This should be dynamically allocated in the future.
  u8 m_buffer[N]{};
};

using RingBuffer = SizedRingBuffer<>;

} // namespace bu
