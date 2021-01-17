if(NOT BUNOS_BUILD_HOST)
    add_library(m.object STATIC math.cpp)
    # Libm might need to be ns
    add_custom_target(
      m
      COMMAND mkdir -p ${BUNOS_FS_ROOT_DIR}/usr/lib/
      COMMAND cp libm.object.a ${BUNOS_FS_ROOT_DIR}/usr/lib/libm.a
      DEPENDS m.object
    )
endif()
