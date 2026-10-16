#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "synthesis::gkernel" for configuration "Release"
set_property(TARGET synthesis::gkernel APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(synthesis::gkernel PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/Release/libgkernel.so"
  IMPORTED_SONAME_RELEASE "libgkernel.so"
  )

list(APPEND _cmake_import_check_targets synthesis::gkernel )
list(APPEND _cmake_import_check_files_for_synthesis::gkernel "${_IMPORT_PREFIX}/lib/Release/libgkernel.so" )

# Import target "synthesis::gds_reader" for configuration "Release"
set_property(TARGET synthesis::gds_reader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(synthesis::gds_reader PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/Release/libgds_reader.so"
  IMPORTED_SONAME_RELEASE "libgds_reader.so"
  )

list(APPEND _cmake_import_check_targets synthesis::gds_reader )
list(APPEND _cmake_import_check_files_for_synthesis::gds_reader "${_IMPORT_PREFIX}/lib/Release/libgds_reader.so" )

# Import target "synthesis::drc_solver" for configuration "Release"
set_property(TARGET synthesis::drc_solver APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(synthesis::drc_solver PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/Release/libdrc_solver.so"
  IMPORTED_SONAME_RELEASE "libdrc_solver.so"
  )

list(APPEND _cmake_import_check_targets synthesis::drc_solver )
list(APPEND _cmake_import_check_files_for_synthesis::drc_solver "${_IMPORT_PREFIX}/lib/Release/libdrc_solver.so" )

# Import target "synthesis::lvs_solver" for configuration "Release"
set_property(TARGET synthesis::lvs_solver APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(synthesis::lvs_solver PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/Release/liblvs_solver.so"
  IMPORTED_SONAME_RELEASE "liblvs_solver.so"
  )

list(APPEND _cmake_import_check_targets synthesis::lvs_solver )
list(APPEND _cmake_import_check_files_for_synthesis::lvs_solver "${_IMPORT_PREFIX}/lib/Release/liblvs_solver.so" )

# Import target "synthesis::core" for configuration "Release"
set_property(TARGET synthesis::core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(synthesis::core PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/Release/libcore.so"
  IMPORTED_SONAME_RELEASE "libcore.so"
  )

list(APPEND _cmake_import_check_targets synthesis::core )
list(APPEND _cmake_import_check_files_for_synthesis::core "${_IMPORT_PREFIX}/lib/Release/libcore.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
