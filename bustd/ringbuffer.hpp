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
  SizedRingBuffer operator=(SizedRingBuffer &&other) = delete;
  // TODO: Maybe these should be a generic interface?
  usize len() const { return m_current_size; }
  usize max_len() const { return N; }
  bool is_full() const { return m_current_size == N; }
  bool is_empty() const { return m_current_size == 0; }
  usize write(const u8 *buffer, const usize length) {
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
    drop(bytes_read);
    return bytes_read;
  }

  usize remaining_space() const { return N - m_current_size; }
  // This was needed for the serial driver
  usize vol_remaining_space() const volatile { return N - m_current_size; }

  // Restrictions: Calling write() invalidates these pointers, and no guarantees
  // are made about the state of these afterwards.
  bool read_nocopy(StringView &first_part, StringView &second_part) {
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

  usize drop(const usize n) {
    const auto to_drop = bu::min(n, m_current_size);
    m_current_size -= to_drop;
    m_buffer_start += to_drop;
    if (m_buffer_start >= N) {
      m_buffer_start -= N;
    }
    return to_drop;
  }

private:
  usize m_buffer_start{0};
  usize m_current_size{0};
  // TODO: This should be dynamically allocated in the future.
  u8 m_buffer[N]{};
};

using RingBuffer = SizedRingBuffer<>;

} // namespace bu
