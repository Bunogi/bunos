set(test_sources ringbuffer.cpp)
foreach(test IN ITEMS ${test_sources})
  bunos_test("${test}" bustd)
endforeach()
