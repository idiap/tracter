#
# Copyright 2015 by Idiap Research Institute, http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, December 2013
#

#
# Try to find Tracter; see
#  https://cmake.org/Wiki/CMake:How_To_Find_Libraries
# Once done this will define
#  TRACTER_FOUND          - System has tracter
#  TRACTER_INCLUDE_DIRS   - The tracter include directory
#  TRACTER_LIBRARIES      - The libraries needed to use Tracter
#  TRACTER_DEFINITIONS    - Compiler switches required for using tracter
#

find_package(PkgConfig)
pkg_check_modules(PC_TRACTER QUIET tracter)

set(TRACTER_DEFINITIONS ${PC_TRACTER_CFLAGS_OTHER})

find_path(
  TRACTER_INCLUDE_DIR Component.h
  HINTS ${PC_TRACTER_INCLUDEDIR} ${PC_TRACTER_INCLUDE_DIRS}
  PATH_SUFFIXES tracter
)

find_library(
  TRACTER_LIBRARY tracter
  HINTS ${PC_TRACTER_LIBDIR} ${PC_TRACTER_LIBRARY_DIRS}
)

set(TRACTER_LIBRARIES ${TRACTER_LIBRARY})
set(TRACTER_INCLUDE_DIRS ${TRACTER_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Tracter DEFAULT_MSG
  TRACTER_LIBRARY TRACTER_INCLUDE_DIR
)

mark_as_advanced(TRACTER_INCLUDE_DIR TRACTER_LIBRARY)
