#pragma once

#include <bustd/assert.hpp>
#include <bustd/math.hpp>
#include <bustd/new.hpp>
#include <bustd/stddef.hpp>
#include <string.h>

#ifdef __IN_KERNEL__
#include <kernel/kmalloc.hpp>
#else
#include <stdlib.h>
#endif

namespace bu {

template <typename T> class Vector {
public:
  Vector() = default;
  Vector(usize preallocated) { set_size(preallocated); }

  Vector(const Vector &other) : Vector(other.len()) {
    if (other.len() == 0) {
      return;
    }

    for (usize i = 0; i < other.len(); i++) {
      new (slot(i)) T(other[i]);
    }
    m_size = other.len();
  }

  Vector &operator=(const Vector &other) {
    *this = Vector(other);
    return *this;
  }

  Vector(Vector &&other) {
    m_size = other.m_size;
    m_capacity = other.m_capacity;
    m_data = other.m_data;

    other.m_size = 0;
    other.m_capacity = 0;
    other.m_data = nullptr;
  }

  Vector &operator=(Vector &&other) {
    *this = Vector(move(other));
    return *this;
  }
  // TODO: List initializer?

  ~Vector() {
    for (usize i = 0; i < m_size; i++) {
      slot(i)->~T();
    }

    if (m_data != nullptr) {
#ifdef __IN_KERNEL__
      kernel::malloc::Allocator::instance()->deallocate(m_data);
#else
      free(m_data);
#endif
    }
  }

  void push(const T &v) { push(T(v)); };

  void push(T &&v) {
    if (m_size + 1 > m_capacity) {
      grow();
    }
    new (slot(m_size)) T(move(v));
    m_size += 1;
  };

  /*
    template <typename... Ts> void emplace(Ts &&... args) {
      *reinterpret_cast<T *>(m_data + m_size * sizeof(T)) =
          T(bu::forward(args)...);
      m_size += 1;
    }
    */

  void pop() {
    ASSERT(m_size > 0);
    m_size--;
    slot(m_size)->~T();
  };

  void pop(T &val) {
    ASSERT(m_size > 0);
    m_size--;
    val = at(m_size);
    slot(m_size)->~T();
  }

  const T &back() const {
    ASSERT(m_size > 0);
    return at(m_size - 1);
  };

  T &back() {
    ASSERT(m_size > 0);
    return at(m_size - 1);
  };

  usize len() const { return m_size; }
  usize capacity() const { return m_capacity; }

  T &operator[](usize index) { return at(index); }
  const T &operator[](usize index) const { return *slot(index); }

  T &at(usize index) { return *slot(index); }
  const T &at(usize index) const { return *slot(index); }

private:
  void grow() {
    const auto new_size = bu::max(m_size * 2, 1lu);
    set_size(new_size);
    ASSERT(new_size > m_size);
  }

  T *slot(usize i) const {
    ASSERT(i < m_capacity);
    return m_data + i;
  }

  void set_size(usize new_size) {
    ASSERT(new_size >= m_size);
#ifdef __IN_KERNEL__
    T *new_data = reinterpret_cast<T *>(
        kernel::malloc::Allocator::instance()->allocate(new_size * sizeof(T)));
#else
    T *new_data = reinterpret_cast<T *>(malloc(new_size * sizeof(T)));
#endif
    if (m_data != nullptr) {
      for (usize i = 0; i < m_size; i++) {
        new (&new_data[i]) T(move(at(i)));
        slot(i)->~T();
      }
#ifdef __IN_KERNEL__
      kernel::malloc::Allocator::instance()->deallocate(m_data);
#else
      free(m_data);
#endif
    }
    m_data = new_data;
    m_capacity = new_size;
  }

  usize m_size{0};
  usize m_capacity{0};
  T *m_data{nullptr};
};
} // namespace bu
