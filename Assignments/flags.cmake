# allolib location

set(allolib_directory allolib)

set (OPENVR_ROOT "C:/Users/Mengyu/source/repos/openvr")

set(OPENVR_INCLUDE_DIR "${OPENVR_ROOT}/headers")
set(OPENVR_LIB "${OPENVR_ROOT}/lib/win64/openvr_api.lib")


if(EXISTS "${OPENVR_LIB}")
  add_definitions(-DBUILD_VR)
  message("Building with OpenVR support")
  
  # other directories to include
  list(APPEND app_include_dirs 
  ${OPENVR_INCLUDE_DIR}
  )

  # other libraries to link
  list(APPEND app_link_libs
  ${OPENVR_LIB}
  )

  # definitions
  list(APPEND app_definitions
  )

  # compile flags
  list(APPEND app_compile_flags
  )

  # linker flags, with `-` in the beginning
  list(APPEND app_linker_flags
  )
endif()


