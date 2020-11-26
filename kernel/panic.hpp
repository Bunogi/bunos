#pragma once

// TODO: find a more appropriate place for these(in bustd?)
#define ASSERT_TRUE(_x)                                                        \
  do {                                                                         \
    if ((_x) != true) {                                                        \
      kernel::panic::panic();                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_NOT_REACHED() kernel::panic::panic();

namespace kernel::panic {
void panic();
}
