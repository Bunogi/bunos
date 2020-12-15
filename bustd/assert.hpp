#ifdef __IN_KERNEL__
#include <kernel/panic.hpp>
#endif

#ifdef __IN_KERNEL__
#define ASSERT(_x)                                                             \
  do {                                                                         \
    if ((_x) != true) {                                                        \
      KERNEL_PANIC("Assertion failed: '" #_x "'");                             \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(_x, _y)                                                      \
  do {                                                                         \
    const auto _lhs = _x;                                                      \
    const auto _rhs = _y;                                                      \
    if (_lhs != _rhs) {                                                        \
      KERNEL_PANIC("Assertion failed: '" #_x "' != '" #_y "'");                \
    }                                                                          \
  } while (0)

#define ASSERT_NOT_REACHED()                                                   \
  do {                                                                         \
    KERNEL_PANIC("Reached unreachable code");                                  \
  } while (0)

#else
#error Non-kernel assert not implemented yet
#endif
