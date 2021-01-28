#pragma once

#include <bustd/assert.hpp>
#include <bustd/stddef.hpp>
#ifdef __IN_KERNEL__
#include <kernel/interruptguard.hpp>
#endif

namespace bu {
enum class Atomic { Copy, No };

namespace __impl {
#ifdef __IN_KERNEL__
constexpr bool supports_atomic = true;
#else
constexpr bool supports_atomic = false;
#endif

} // namespace __impl

template <typename T, Atomic ATOMICITY = Atomic::No> class RcBase {
public:
  static_assert(ATOMICITY == Atomic::Copy ? __impl::supports_atomic : true,
                "Atomic Rc not supported for this platform");
  RcBase() = delete;
  RcBase(const RcBase &other) : m_contents(other.m_contents) {
    *this = other;
    increase_count();
  }
  ~RcBase() {
    if (m_valid) {
      decrease_count();
    }
  }
  RcBase &operator=(const RcBase &other) {
    m_contents = other.m_contents;
    return *this;
  }
  RcBase &operator=(RcBase &&other) {
    m_contents = other.m_contents;
    other.m_valid = false;
  }
  RcBase(RcBase &&other) { *this = other; }

  explicit RcBase(T &&data) { m_contents = new Contents{forward(data), 1}; }

  usize count() const {
    if constexpr (ATOMICITY == Atomic::Copy) {
#ifdef __IN_KERNEL__
      const kernel::InterruptGuard guard;
#endif
      return m_contents->count;
    } else {
      return m_contents->count;
    }
  }

  // These are
  const T &operator*() const {
    ASSERT(m_contents);
    return m_contents->data;
  }

  T &operator*() {
    ASSERT(m_contents);
    return m_contents->data;
  };

private:
  void delete_if_last() {
    if (m_contents == 0) {
      delete m_contents;
    }
  }

  void increase_count() {
    if constexpr (ATOMICITY == Atomic::Copy) {
#ifdef __IN_KERNEL__
      const kernel::InterruptGuard guard;
#endif
      m_contents->count++;
    } else {
      m_contents->count++;
    }
  }

  void decrease_count() {
    if constexpr (ATOMICITY == Atomic::Copy) {
#ifdef __IN_KERNEL__
      const kernel::InterruptGuard guard;
#endif
      m_contents->count--;
      delete_if_last();
    } else {
      m_contents->count--;
      delete_if_last();
    }
  }
  // Slight optimization: Allocate one struct at a time
  struct Contents {
    T data;
    usize count;
  };
  Contents *m_contents{nullptr};
  bool m_valid{true};
};

template <typename T> class AtomicRc : public RcBase<T, Atomic::Copy> {
  using RcBase<T, Atomic::Copy>::RcBase;
};
template <typename T> class Rc : public RcBase<T, Atomic::No> {
  using RcBase<T, Atomic::No>::RcBase;
};

template <typename T, typename... Ts>
inline Rc<T> create_refcounted(Ts &&... args) {
  return Rc<T>(T(forward(args...)));
}

template <typename T, typename... Ts>
inline AtomicRc<T> create_atomic_refcounted(Ts &&... args) {
  return AtomicRc<T>(T(forward(args)...));
}

} // namespace bu
