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

function(bunos_lib)
  #FIXME: Remove KERNEL_ONLY when we have a userspace
  cmake_parse_arguments(
    LIB
    "KERNEL_SPECIFIC_BUILD_REQUIRED;HOST_DISABLE;KERNEL_ONLY"
    "NAME"
    "SOURCES"
    ${ARGN})

  if (LIB_KERNEL_SPECIFIC_BUILD_REQUIRED AND NOT BUNOS_BUILD_HOST)
    add_library(${LIB_NAME}-kernel STATIC ${LIB_SOURCES})
    target_compile_definitions(${LIB_NAME}-kernel PRIVATE __IN_KERNEL__=1)
    set_target_properties(${LIB_NAME}-kernel PROPERTIES COMPILE_FLAGS "${KERNEL_BUILD_FLAGS}")
  endif()

  if (BUNOS_BUILD_HOST)
    if (NOT LIB_HOST_DISABLE)
      add_library(${LIB_NAME} STATIC ${LIB_SOURCES})
    endif()
  elseif(NOT LIB_KERNEL_ONLY)
    add_library(${LIB_NAME} STATIC ${LIB_SOURCES})
  endif()
endfunction()

add_compile_definitions(__IS_X86__=1)

project(bunos CXX ASM)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Os")
set(WARNING_FLAGS "-Wall -Wextra -Werror=return-type -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -fno-omit-frame-pointer -fno-exceptions -fno-rtti")

enable_testing()

include_directories(${CMAKE_SOURCE_DIR} libc) #TODO: remove libc from this when able

set(KERNEL_BUILD_FLAGS "-nostdlib -ffreestanding")

add_subdirectory(bustd)
add_subdirectory(libc)
add_subdirectory(libraries)

if(NOT BUNOS_BUILD_HOST)
  add_subdirectory(kernel)
endif()

