#
# Written by cfind.sh
# Part of lube, https://github.com/pgarner/libube
# See also https://cmake.org/Wiki/CMake:How_To_Find_Libraries
#
# The following should end up defined: 
#  SNDFILE_FOUND          - System has SndFile
#  SNDFILE_INCLUDE_DIR    - The SndFile include directories
#  SNDFILE_LIBRARIES      - The libraries needed to use SndFile
#  SNDFILE_DEFINITIONS    - Compiler switches required for using SndFile
#
find_package(PkgConfig)
pkg_check_modules(PC_SNDFILE QUIET sndfile)

set(SNDFILE_DEFINITIONS ${PC_SNDFILE_CFLAGS_OTHER})

find_path(
  SNDFILE_INCLUDE_DIR sndfile.h
  HINTS ${PC_SNDFILE_INCLUDEDIR} ${PC_SNDFILE_INCLUDE_DIRS}
  PATH_SUFFIXES sndfile
)

find_library(
  SNDFILE_LIBRARY sndfile  # Without the lib- prefix
  HINTS ${PC_SNDFILE_LIBDIR} ${PC_SNDFILE_LIBRARY_DIRS}
)

set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARY})  # Can add ${CMAKE_DL_LIBS}
set(SNDFILE_INCLUDE_DIRS ${SNDFILE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SndFile DEFAULT_MSG
  SNDFILE_LIBRARY SNDFILE_INCLUDE_DIR
)

mark_as_advanced(SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)
