#pragma once
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
  OwnedPtr(OwnedPtr &&other) : m_data(other.m_data) { other.m_valid = false; }
  OwnedPtr &operator=(OwnedPtr &&other) {
    other.m_valid = false;
    m_data = other.m_data;
    return *this;
  }

  explicit OwnedPtr(T *data) : m_data(data) {}
  explicit OwnedPtr(nullptr_t) : m_data(nullptr), m_valid(false) {}

  ~OwnedPtr() {
    if (m_valid) {
      delete m_data;
    }
  }

  const T *operator->() const { return m_data; }
  T *operator->() { return m_data; }

  bool is_null() const { return m_data == nullptr; }
  operator bool() const { return !is_null(); }

private:
  T *m_data;
  bool m_valid{true};
};

template <typename T, typename... Ts>
inline OwnedPtr<T> create_owned(Ts &&... args) {
  return OwnedPtr(new T(forward(args)...));
}

} // namespace bu
