set(test_sources
  bitfield.cpp
  list.cpp
  optional.cpp
  ownedptr.cpp
  rc.cpp
  ringbuffer.cpp
  stringview.cpp
  vector.cpp
)
foreach(test IN ITEMS ${test_sources})
  bunos_test("${test}" bustd)
endforeach()
