#pragma once

#include <bustd/assert.hpp>
#include <bustd/function.hpp>
#include <bustd/math.hpp>
#include <bustd/new.hpp>
#include <bustd/stddef.hpp>
#include <stdlib.h>
#include <string.h>

namespace bu {

template <typename T> class Vector {
public:
  Vector() = default;
  Vector(usize preallocated) {
    if (preallocated != 0) {
      set_size(preallocated);
    }
  }

  Vector(const Vector &other) : Vector(other.len()) { *this = other; }

  Vector &operator=(const Vector &other) {
    clear();

    for (usize i = 0; i < other.len(); i++) {
      push(other[i]);
    }
    return *this;
  }

  Vector(Vector &&other) { *this = forward(other); }

  Vector &operator=(Vector &&other) {
    if (m_data != nullptr) {
      free(m_data);
    }

    m_size = other.m_size;
    m_capacity = other.m_capacity;
    m_data = other.m_data;

    other.m_size = 0;
    other.m_capacity = 0;
    other.m_data = nullptr;

    return *this;
  }
  // TODO: List initializer?

  ~Vector() {
    for (usize i = 0; i < m_size; i++) {
      slot(i)->~T();
    }

    if (m_data != nullptr) {
      free(m_data);
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

  // Set `upto` elements of the vector to `v`.
  // Any existing values are updated.
  void fill(const T &v, usize upto) {
    for (usize i = 0; i < len(); i++) {
      at(i) = v;
    }

    for (usize i = len(); i < upto; i++) {
      push(v);
    }
  }

  T *data() { return m_data; }

  const T *data() const { return m_data; }

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
    val = *slot(m_size);
    slot(m_size)->~T();
  }

  const T &back() const { return *safe_slot(m_size - 1); };

  T &back() { return *safe_slot(m_size - 1); };

  usize len() const { return m_size; }
  usize capacity() const { return m_capacity; }
  bool is_empty() const { return m_size == 0; }

  void clear() {
    while (!is_empty()) {
      pop();
    }
  }

  void resize_to_fit() { set_size(m_size); }

  T &operator[](usize index) { return *safe_slot(index); }
  const T &operator[](usize index) const { return *safe_slot(index); }

  T &at(usize index) { return *safe_slot(index); }
  const T &at(usize index) const { return *safe_slot(index); }

  // FIXME: insert() is probably good to have too
  void remove(usize index) {
    ASSERT(index < m_size);

    slot(index)->~T();

    for (usize i = index; i + 1 < m_size; i++) {
      slot(i)->~T();
      new (slot(i)) T(move(at(i + 1)));
    }
    m_size--;
  }

  void remove_if(Function<bool(const T &)> f) {
    for (usize i = 0; i < m_size; i++) {
      if (f(at(i))) {
        remove(i);
      }
    }
  }

  class Iterator {
  public:
    Iterator operator++() const {
      if (m_reverse) {
        m_index--;
      } else {
        m_index++;
      }
      return *this;
    }
    Iterator operator++(int) const { return operator++(); }
    T &operator*() { return m_parent[m_index]; }
    const T &operator*() const { return m_parent[m_index]; }
    bool operator!=(const Iterator &other) const {
      return m_index != other.m_index;
    }

  private:
    friend class Vector<T>;
    Iterator(Vector<T> &parent, usize index, bool reverse)
        : m_parent(parent), m_index(index), m_reverse(reverse) {}
    Vector<T> &m_parent;
    mutable usize m_index;
    bool m_reverse;
  };

  Iterator begin() { return Iterator(*this, 0, false); }

  Iterator rbegin() { return Iterator(*this, m_size - 1, false); }

  Iterator end() { return Iterator(*this, m_size, false); }
  Iterator rend() { return Iterator(*this, -1, false); }

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

  T *safe_slot(usize i) const {
    ASSERT(i < m_size);
    return m_data + i;
  }

  void set_size(usize new_size) {
    ASSERT(new_size >= m_size);
    T *new_data = reinterpret_cast<T *>(malloc(new_size * sizeof(T)));
    if (m_data != nullptr) {
      for (usize i = 0; i < m_size; i++) {
        new (&new_data[i]) T(move(at(i)));
        slot(i)->~T();
      }
      free(m_data);
    }
    m_data = new_data;
    m_capacity = new_size;
  }

  usize m_size{0};
  usize m_capacity{0};
  T *m_data{nullptr};
};
} // namespace bu
