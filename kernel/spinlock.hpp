#pragma once

#include <bustd/macros.hpp>

namespace kernel {
class SpinLock {
  BU_NOCOPY(SpinLock)
  BU_NOMOVE(SpinLock)
public:
  SpinLock();

  class Guard {
    BU_NOCOPY(Guard)
    BU_NOMOVE(Guard)
    friend class SpinLock;

  public:
    ~Guard();

  private:
    Guard(SpinLock &parent);

    SpinLock &m_parent;
  };
  friend class Guard;

  Guard lock();
  bool is_locked();

private:
  void unlock();
  volatile bool m_valid;
  volatile bool m_locked;
};
} // namespace kernel
