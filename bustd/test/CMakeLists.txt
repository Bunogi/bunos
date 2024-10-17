#FIXME: Userland malloc :^)
set(test_sources_target
  bitfield.cpp
  optional.cpp
  ringbuffer.cpp
  scopeguard.cpp
  stringview.cpp
)


set(test_sources
  algorithm.cpp
  list.cpp
  ownedptr.cpp
  vector.cpp
)


foreach(test IN ITEMS ${test_sources_target})
  bunos_test(bustd ${test} bustd)
endforeach()

if (BUNOS_BUILD_HOST)
  foreach(test IN ITEMS ${test_sources})
    bunos_test(bustd ${test} bustd)
  endforeach()
endif()
