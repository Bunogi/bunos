cmake_minimum_required(VERSION 3.18.2)

if(NOT BUNOS_BUILD_HOST)
  set(CMAKE_C_COMPILER_WORKS 1)
  set(CMAKE_CXX_COMPILER_WORKS 1)
  set(CMAKE_TOOLCHAIN_FILE cross.cmake)
  add_compile_definitions(__BUNOS__) #TODO: should be done with os-specific compiler
endif()

function(bunos_test FILE LIBS)
  if(BUNOS_BUILD_HOST)
    set(testname ${FILE}.test)
    add_executable(${testname} ${FILE})
    target_link_libraries(${testname} bustd ${LIBS})
    add_test(NAME ${testname} COMMAND ${testname})
  else()
    message("Testing from inside bunos is not supported yet")
  endif()
endfunction()

add_compile_definitions(__IS_X86__=1)

project(bunos CXX ASM)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fno-omit-frame-pointer")

enable_testing()

include_directories(${CMAKE_SOURCE_DIR} libc) #TODO: remove libc from this when able

set(KERNEL_BUILD_FLAGS "-nostdlib -ffreestanding -fno-exceptions -fno-rtti")

add_subdirectory(bustd)
add_subdirectory(libc)
add_subdirectory(libraries)

if(NOT BUNOS_BUILD_HOST)
  add_subdirectory(kernel)
endif()

