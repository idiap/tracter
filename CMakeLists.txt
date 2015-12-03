#
# CMake file for Tracter
#
# Phil Garner
# March 2009
#

# Version 2.4 might work, but it won't find static libraries with odd names
cmake_minimum_required(VERSION 2.6)

# CMake used to complain without this
#if(COMMAND cmake_policy)
#  cmake_policy(SET CMP0003 NEW)
#endif(COMMAND cmake_policy)

# Top level project information
project(tracter)
set(VERSION 1.1)

# Find boost, get rid of the version number
find_package(Boost COMPONENTS system filesystem REQUIRED)
string(REGEX REPLACE "'.'so.*" ".so" Boost_LIBRARIES "${Boost_LIBRARIES}")
message(STATUS "Boost lib is ${Boost_LIBRARIES}")

# Find other packages
#
# To build *totally* static, comment out ALSA, SndFile, PulseAudio and
# RtAudio.  Pulse on Ubuntu doesn't have static libraries; the others
# use dynamic loading (maybe...)
#
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(KissFFT REQUIRED)
find_package(Torch3)
find_package(HTK)
find_package(BSAPI)
find_package(ALSA)
find_package(SndFile)
find_package(PulseAudio)

find_package(LibResample)
find_package(RtAudio)
find_package(SPTK)

add_subdirectory(bin)
add_subdirectory(tracter)

# Configure other files
configure_file(Doxyfile.in Doxyfile)

# This one is for binary distributions
include(CPack)

# For source, it's *much* better to use git
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${VERSION})
add_custom_target(dist
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
    | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
