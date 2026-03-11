#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "synthesis::gkernel" for configuration "Debug"
set_property(TARGET synthesis::gkernel APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(synthesis::gkernel PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/Debug/libgkerneld.so"
  IMPORTED_SONAME_DEBUG "libgkerneld.so"
  )

list(APPEND _cmake_import_check_targets synthesis::gkernel )
list(APPEND _cmake_import_check_files_for_synthesis::gkernel "${_IMPORT_PREFIX}/lib/Debug/libgkerneld.so" )

# Import target "synthesis::gds_reader" for configuration "Debug"
set_property(TARGET synthesis::gds_reader APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(synthesis::gds_reader PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/Debug/libgds_readerd.so"
  IMPORTED_SONAME_DEBUG "libgds_readerd.so"
  )

list(APPEND _cmake_import_check_targets synthesis::gds_reader )
list(APPEND _cmake_import_check_files_for_synthesis::gds_reader "${_IMPORT_PREFIX}/lib/Debug/libgds_readerd.so" )

# Import target "synthesis::drc_solver" for configuration "Debug"
set_property(TARGET synthesis::drc_solver APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(synthesis::drc_solver PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/Debug/libdrc_solverd.so"
  IMPORTED_SONAME_DEBUG "libdrc_solverd.so"
  )

list(APPEND _cmake_import_check_targets synthesis::drc_solver )
list(APPEND _cmake_import_check_files_for_synthesis::drc_solver "${_IMPORT_PREFIX}/lib/Debug/libdrc_solverd.so" )

# Import target "synthesis::lvs_solver" for configuration "Debug"
set_property(TARGET synthesis::lvs_solver APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(synthesis::lvs_solver PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/Debug/liblvs_solverd.so"
  IMPORTED_SONAME_DEBUG "liblvs_solverd.so"
  )

list(APPEND _cmake_import_check_targets synthesis::lvs_solver )
list(APPEND _cmake_import_check_files_for_synthesis::lvs_solver "${_IMPORT_PREFIX}/lib/Debug/liblvs_solverd.so" )

# Import target "synthesis::core" for configuration "Debug"
set_property(TARGET synthesis::core APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(synthesis::core PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/Debug/libcored.so"
  IMPORTED_SONAME_DEBUG "libcored.so"
  )

list(APPEND _cmake_import_check_targets synthesis::core )
list(APPEND _cmake_import_check_files_for_synthesis::core "${_IMPORT_PREFIX}/lib/Debug/libcored.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
