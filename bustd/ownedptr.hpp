#pragma once
#include <bustd/assert.hpp>
#include <bustd/macros.hpp>
#include <bustd/stddef.hpp>
#include <bustd/type_traits.hpp>

namespace bu {
template <typename T> class OwnedPtr {
  BU_NOCOPY(OwnedPtr)

  static_assert(!is_same<nullptr_t, T>::value,
                "Cannot make a nullptr_t pointer");

public:
  OwnedPtr() = delete;
  OwnedPtr(OwnedPtr &&other) { *this = forward(other); }

  OwnedPtr &operator=(OwnedPtr &&other) {
    dealloc();
    m_data = other.m_data;

    other.deactivate();
    return *this;
  }

  explicit OwnedPtr(T *data) : m_data(data) {}
  explicit OwnedPtr(nullptr_t) : m_data(nullptr) {}

  ~OwnedPtr() { delete m_data; }

  const T *operator->() const {
    ASSERT(!is_null());
    return m_data;
  }
  T *operator->() {
    ASSERT(!is_null());
    return m_data;
  }
  const T &operator*() const {
    ASSERT(!is_null());
    return *m_data;
  }
  T &operator*() {
    ASSERT(!is_null());
    return *m_data;
  }

  bool is_null() const { return m_data == nullptr; }
  operator bool() const { return !is_null(); }

  T *get() {
    ASSERT(!is_null());
    return m_data;
  }
  const T *get() const {
    ASSERT(!is_null());
    return m_data;
  }

private:
  void dealloc() {
    if (m_data) {
      delete m_data;
    }
    deactivate();
  }
  void deactivate() { m_data = 0; }

  T *m_data{nullptr};
};

template <typename T, typename... Ts>
inline OwnedPtr<T> create_owned(Ts &&...args) {
  auto s = OwnedPtr(new T(forward(args)...));
  ASSERT(s);
  return move(s);
}

} // namespace bu
