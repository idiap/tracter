#
# Copyright 2015 by Idiap Research Institute
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, November 2015
#
# ...but basically copied from FindSndFile in libube, in turn from the examples
# on the web.
#

#
# Try to find RtAudio
# Once done this will define
#  RTAUDIO_FOUND          - System has RtAudio
#  RTAUDIO_INCLUDE_DIR    - The RtAudio include directories
#  RTAUDIO_LIBRARIES      - The libraries needed to use RtAudio
#  RTAUDIO_DEFINITIONS    - Compiler switches required for using RtAudio
#  RTAUDIO_VERSION_STRING - the version of RtAudio found
#

find_package(PkgConfig)
pkg_check_modules(PC_RTAUDIO QUIET librtaudio)

set(RTAUDIO_DEFINITIONS ${PC_RTAUDIO_CFLAGS_OTHER})
set(RTAUDIO_VERSION_STRING ${PC_RTAUDIO_VERSION})

find_path(
  RTAUDIO_INCLUDE_DIR RtAudio.h
  HINTS ${PC_RTAUDIO_INCLUDEDIR} ${PC_RTAUDIO_INCLUDE_DIRS}
)

find_library(
  RTAUDIO_LIBRARY NAMES rtaudio
  HINTS ${PC_RTAUDIO_LIBDIR} ${PC_RTAUDIO_LIBRARY_DIRS}
)

set(RTAUDIO_LIBRARIES ${RTAUDIO_LIBRARY})
set(RTAUDIO_INCLUDE_DIRS ${RTAUDIO_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  RtAudio
  REQUIRED_VARS RTAUDIO_LIBRARY RTAUDIO_INCLUDE_DIR
  VERSION_VAR RTAUDIO_VERSION_STRING
)

mark_as_advanced(RTAUDIO_INCLUDE_DIR RTAUDIO_LIBRARY)
