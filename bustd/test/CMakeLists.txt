set(test_sources
  bitfield.cpp
  list.cpp
  ownedptr.cpp
  rc.cpp
  ringbuffer.cpp
  vector.cpp
)
foreach(test IN ITEMS ${test_sources})
  bunos_test("${test}" bustd)
endforeach()
