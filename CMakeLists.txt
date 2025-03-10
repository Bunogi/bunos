cmake_minimum_required(VERSION 3.18.2)

if(NOT BUNOS_BUILD_HOST)
  set(CMAKE_TOOLCHAIN_FILE cross.cmake)
endif()

function(bunos_test COMPONENT FILE LIBS)
  set(testname test_${COMPONENT}_${FILE})
  add_executable(${testname} ${FILE})
  target_link_libraries(${testname} bustd ${LIBS} c m stdc++)
  if(BUNOS_BUILD_HOST)
    add_test(NAME ${testname} COMMAND ${testname})
  else()
    install(TARGETS ${testname} DESTINATION ${BUNOS_INSTALL_DIR}/bin/test)
  endif()
endfunction()

function(bunos_lib)
  cmake_parse_arguments(
    LIB
    "KERNEL_SPECIFIC_BUILD_REQUIRED;HOST_DISABLE"
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
  else()
    add_library(${LIB_NAME} STATIC ${LIB_SOURCES})
  endif()
endfunction()

function(bunos_app)
  cmake_parse_arguments(
    APP
    "HOST_ENABLE"
    "NAME"
    "SOURCES;LIBS"
    ${ARGN}
  )

  set(SHOULD_BUILD TRUE)
  if (BUNOS_BUILD_HOST AND NOT APP_HOST_ENABLE)
    set(SHOULD_BUILD FALSE)
  endif()

  if (SHOULD_BUILD)
    add_executable(${APP_NAME} ${APP_SOURCES})
    target_link_libraries(${APP_NAME} PRIVATE ${APP_LIBS})
    if (NOT BUNOS_BUILD_HOST)
      target_link_libraries(${APP_NAME} PRIVATE ${APP_LIBS} c.object)
      # FIXME: stdc++ should be removed from this list
      add_dependencies(${APP_NAME} c m stdc++)
      install(TARGETS ${APP_NAME} DESTINATION ${BUNOS_INSTALL_DIR}/bin)
    endif()
  endif()
endfunction()

add_compile_definitions(__IS_X86__=1)

project(bunos C CXX ASM)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(WARNING_FLAGS "-Wall -Wextra -pedantic -Wno-analyzer-possible-null-dereference")

set(WARNING_ERRORS "")
list(APPEND WARNING_ERRORS "return-type")
list(APPEND WARNING_ERRORS "switch")
list(APPEND WARNING_ERRORS "class-memaccess")
list(APPEND WARNING_ERRORS "unused-function")
list(APPEND WARNING_ERRORS "uninitialized")
list(APPEND WARNING_ERRORS "nonnull")
list(APPEND WARNING_ERRORS "delete-non-virtual-dtor")
list(APPEND WARNING_ERRORS "address")

foreach(i ${WARNING_ERRORS})
  set(WARNING_ERROR_FLAGS "${WARNING_ERROR_FLAGS} -Werror=${i}")
endforeach()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${WARNING_ERROR_FLAGS} -fno-omit-frame-pointer -fno-exceptions -fno-rtti")

if(BUNOS_BUILD_HOST)
  add_compile_options(-fsanitize=address -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

enable_testing()

set(BUNOS_INSTALL_DIR ${CMAKE_SOURCE_DIR}/root)
set(BUNOS_FS_ROOT_DIR ${CMAKE_SOURCE_DIR}/fsroot)

include_directories(${CMAKE_SOURCE_DIR} libraries)

set(KERNEL_BUILD_FLAGS "-nostdlib -ffreestanding")

add_subdirectory(bustd)
add_subdirectory(libraries)
add_subdirectory(coreutils)

if(NOT BUNOS_BUILD_HOST)
  add_subdirectory(kernel)
  add_subdirectory(libc)

  add_custom_target(
    image
    COMMAND ${CMAKE_SOURCE_DIR}/build_disk_image.sh disk.img ${BUNOS_FS_ROOT_DIR} ${BUNOS_INSTALL_DIR}
    DEPENDS kernel.bin kernel.sym ${BUNOS_ROOT_DIR}
  )
endif()
