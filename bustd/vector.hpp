#pragma once

#include <bustd/assert.hpp>
#include <bustd/function.hpp>
#include <bustd/math.hpp>
#include <bustd/new.hpp>
#include <bustd/stddef.hpp>
#include <bustd/type_traits.hpp>
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

  auto operator=(const Vector &other) -> Vector & {
    clear();

    for (usize i = 0; i < other.len(); i++) {
      push(other[i]);
    }
    return *this;
  }

  Vector(Vector &&other) { *this = forward(other); }

  auto operator=(Vector &&other) -> Vector & {
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

  auto data() -> T * { return m_data; }

  [[nodiscard]] auto data() const -> const T * { return m_data; }

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

  [[nodiscard]] auto back() const -> const T & {
    return *safe_slot(m_size - 1);
  };

  auto back() -> T & { return *safe_slot(m_size - 1); };

  [[nodiscard]] auto len() const -> usize { return m_size; }
  [[nodiscard]] auto capacity() const -> usize { return m_capacity; }
  [[nodiscard]] auto is_empty() const -> bool { return m_size == 0; }

  void clear() {
    while (!is_empty()) {
      pop();
    }
  }

  void resize_to_fit() { set_size(m_size); }

  auto operator[](usize index) -> T & { return *safe_slot(index); }
  auto operator[](usize index) const -> const T & { return *safe_slot(index); }

  auto at(usize index) -> T & { return *safe_slot(index); }
  [[nodiscard]] auto at(usize index) const -> const T & {
    return *safe_slot(index);
  }

  // FIXME: insert() is probably good to have too
  void remove(usize index) {
    ASSERT(index < m_size);

    slot(index)->~T();

    if constexpr (is_trivially_coyable<T>) {
      if (index + 1 < m_size) {
        memmove(slot(index), slot(index + 1), (m_size - index - 1) * sizeof(T));
      }
    } else {
      for (usize i = index; i < m_size; i++) {
        *slot(i) = *slot(i + 1);
      }
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
    auto operator++() const -> Iterator {
      if (m_reverse) {
        m_index--;
      } else {
        m_index++;
      }
      return *this;
    }
    auto operator++(int) const -> Iterator {
      Iterator tmp(*this);
      operator++();
      return tmp;
    }
    auto operator*() -> T & { return m_parent[m_index]; }
    auto operator*() const -> const T & { return m_parent[m_index]; }
    auto operator!=(const Iterator &other) const -> bool {
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

  class ConstIterator {
  public:
    auto operator++() const -> ConstIterator {
      if (m_reverse) {
        m_index--;
      } else {
        m_index++;
      }
      return *this;
    }
    auto operator++(int) const -> ConstIterator {
      ConstIterator tmp(*this);
      operator++();
      return tmp;
    }
    auto operator*() const -> const T & { return m_parent[m_index]; }
    auto operator!=(const ConstIterator &other) const -> bool {
      return m_index != other.m_index;
    }

  private:
    friend class Vector<T>;
    ConstIterator(const Vector<T> &parent, usize index, bool reverse)
        : m_parent(parent), m_index(index), m_reverse(reverse) {}
    const Vector<T> &m_parent;
    mutable usize m_index;
    bool m_reverse;
  };

  auto begin() -> Iterator { return Iterator(*this, 0, false); }
  auto rbegin() -> Iterator { return Iterator(*this, m_size - 1, false); }
  auto end() -> Iterator { return Iterator(*this, m_size, false); }
  auto rend() -> Iterator { return Iterator(*this, -1, false); }

  [[nodiscard]] auto begin() const -> ConstIterator {
    return ConstIterator(*this, 0, false);
  }
  [[nodiscard]] auto rbegin() const -> ConstIterator {
    return ConstIterator(*this, m_size - 1, false);
  }
  [[nodiscard]] auto end() const -> ConstIterator {
    return ConstIterator(*this, m_size, false);
  }
  [[nodiscard]] auto rend() const -> ConstIterator {
    return ConstIterator(*this, -1, false);
  }

private:
  void grow() {
    const auto new_size = bu::max(m_size * 2, static_cast<usize>(1));
    set_size(new_size);
    ASSERT(new_size > m_size);
  }

  [[nodiscard]] auto slot(usize i) const -> T * {
    ASSERT(i < m_capacity);
    return m_data + i;
  }

  [[nodiscard]] auto safe_slot(usize i) const -> T * {
    ASSERT(i < m_size);
    return m_data + i;
  }

  void set_size(usize new_size) {
    ASSERT(new_size >= m_size);
    T *new_data = reinterpret_cast<T *>(malloc(new_size * sizeof(T)));
    ASSERT(new_data);
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
