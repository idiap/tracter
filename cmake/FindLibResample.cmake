#
# Written by cfind.sh
# Part of lube, https://github.com/pgarner/libube
# See also https://cmake.org/Wiki/CMake:How_To_Find_Libraries
#
# The following should end up defined: 
#  LIBRESAMPLE_FOUND          - System has LibResample
#  LIBRESAMPLE_INCLUDE_DIR    - The LibResample include directories
#  LIBRESAMPLE_LIBRARIES      - The libraries needed to use LibResample
#  LIBRESAMPLE_DEFINITIONS    - Compiler switches required for using LibResample
#
find_package(PkgConfig)
pkg_check_modules(PC_LIBRESAMPLE QUIET libresample)

set(LIBRESAMPLE_DEFINITIONS ${PC_LIBRESAMPLE_CFLAGS_OTHER})

find_path(
  LIBRESAMPLE_INCLUDE_DIR libresample.h
  HINTS ${PC_LIBRESAMPLE_INCLUDEDIR} ${PC_LIBRESAMPLE_INCLUDE_DIRS}
  PATH_SUFFIXES libresample
)

find_library(
  LIBRESAMPLE_LIBRARY resample  # Without the lib- prefix
  HINTS ${PC_LIBRESAMPLE_LIBDIR} ${PC_LIBRESAMPLE_LIBRARY_DIRS}
)

set(LIBRESAMPLE_LIBRARIES ${LIBRESAMPLE_LIBRARY})  # Can add ${CMAKE_DL_LIBS}
set(LIBRESAMPLE_INCLUDE_DIRS ${LIBRESAMPLE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibResample DEFAULT_MSG
  LIBRESAMPLE_LIBRARY LIBRESAMPLE_INCLUDE_DIR
)

mark_as_advanced(LIBRESAMPLE_INCLUDE_DIR LIBRESAMPLE_LIBRARY)
