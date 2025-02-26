#pragma once

#include "macros.hpp"
#include "type_traits.hpp"

namespace bu {
template <class T> class ScopeGuard {
  BU_NOCOPY(ScopeGuard)
  BU_NOMOVE(ScopeGuard)

public:
  ScopeGuard(T f) : m_func(move(f)) {};
  ~ScopeGuard() {
    if (m_armed) {
      m_func();
    }
  };

  auto disarm() -> bool {
    const bool temp = m_armed;
    m_armed = false;
    return temp;
  };
  auto rearm() -> bool {
    const bool temp = m_armed;
    m_armed = true;
    return temp;
  };
  [[nodiscard]] auto armed() const -> bool { return m_armed; };

private:
  bool m_armed{true};
  T m_func;
};

template <class T> class LockedScopeGuard {
  BU_NOCOPY(LockedScopeGuard);
  BU_NOMOVE(LockedScopeGuard);

public:
  LockedScopeGuard(T f) : m_func(move(f)) {};
  ~LockedScopeGuard() { m_func(); };

private:
  T m_func;
};

} // namespace bu
