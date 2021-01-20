set(test_sources ringbuffer.cpp bitfield.cpp vector.cpp ownedptr.cpp list.cpp)
foreach(test IN ITEMS ${test_sources})
  bunos_test("${test}" bustd)
endforeach()
