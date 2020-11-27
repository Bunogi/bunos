#ifdef __IN_KERNEL__
#include <kernel/panic.hpp>
#endif

#ifdef __IN_KERNEL__
#define ASSERT_TRUE(_x)                                                        \
  do {                                                                         \
    if ((_x) != true) {                                                        \
      kpanic();                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_NOT_REACHED()                                                   \
  do {                                                                         \
    kpanic();                                                                  \
  } while (0)

#else
#error Non-kernel assert not implemented yet
#endif
