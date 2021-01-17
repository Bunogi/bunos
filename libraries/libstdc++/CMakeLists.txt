# FIXME: This whole file is a nasty hack to be able to link executables
# until we can successfully port libstdc++ directly
if(NOT BUNOS_BUILD_HOST)
    add_library(stdc++.object STATIC blank.cpp)
    add_custom_target(
      stdc++
      COMMAND mkdir -p ${BUNOS_FS_ROOT_DIR}/usr/lib/
      COMMAND cp libstdc++.object.a ${BUNOS_FS_ROOT_DIR}/usr/lib/libstdc++.a
      DEPENDS stdc++.object
    )
endif()
