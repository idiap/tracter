#
# Copyright 2015 by Idiap Research Institute
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, November 2015
#

find_path(
  LIBRESAMPLE_INCLUDE_DIR libresample.h
)

find_library(
  LIBRESAMPLE_LIBRARY NAMES libresample.so libresample.a
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  libresample
  REQUIRED_VARS LIBRESAMPLE_LIBRARY LIBRESAMPLE_INCLUDE_DIR
)

if(LIBRESAMPLE_FOUND)
  set(LIBRESAMPLE_LIBRARIES ${LIBRESAMPLE_LIBRARY})
  set(LIBRESAMPLE_INCLUDE_DIRS ${LIBRESAMPLE_INCLUDE_DIR})
endif()

mark_as_advanced(LIBRESAMPLE_INCLUDE_DIR LIBRESAMPLE_LIBRARY)
