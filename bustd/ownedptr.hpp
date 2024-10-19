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

  auto operator=(OwnedPtr &&other) -> OwnedPtr & {
    dealloc();
    m_data = other.m_data;

    other.deactivate();
    return *this;
  }

  explicit OwnedPtr(T *data) : m_data(data) {}
  explicit OwnedPtr(nullptr_t) : m_data(nullptr) {}

  ~OwnedPtr() { delete m_data; }

  auto operator->() const -> const T * {
    ASSERT(!is_null());
    return m_data;
  }
  auto operator->() -> T * {
    ASSERT(!is_null());
    return m_data;
  }
  auto operator*() const -> const T & {
    ASSERT(!is_null());
    return *m_data;
  }
  auto operator*() -> T & {
    ASSERT(!is_null());
    return *m_data;
  }

  [[nodiscard]] auto is_null() const -> bool { return m_data == nullptr; }
  operator bool() const { return !is_null(); }

  auto get() -> T * {
    ASSERT(!is_null());
    return m_data;
  }
  [[nodiscard]] auto get() const -> const T * {
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
inline auto create_owned(Ts &&...args) -> OwnedPtr<T> {
  auto s = OwnedPtr(new T(forward(args)...));
  ASSERT(s);
  return move(s);
}

} // namespace bu
