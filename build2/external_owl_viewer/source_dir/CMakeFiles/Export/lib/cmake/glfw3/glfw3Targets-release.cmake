#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glfw" for configuration "Release"
set_property(TARGET glfw APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glfw PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libglfw.so.3.4"
  IMPORTED_SONAME_RELEASE "libglfw.so.3"
  )

list(APPEND _IMPORT_CHECK_TARGETS glfw )
list(APPEND _IMPORT_CHECK_FILES_FOR_glfw "${_IMPORT_PREFIX}/lib/libglfw.so.3.4" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
