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
set(VERSION 0.6.0)

# Find packages
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(KissFFT REQUIRED)
find_package(Torch3)
find_package(HTK)
find_package(BSAPI)
find_package(ALSA)
find_package(SndFile)
find_package(LibResample)
find_package(RtAudio)
find_package(SPTK)

# Configure other files
configure_file(Doxyfile.in Doxyfile)
configure_file(tracter.pc.in tracter.pc)

add_subdirectory(src)

# This one is for binary distributions
include(CPack)

# For source, it's *much* better to use git
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${VERSION})
add_custom_target(dist
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
    | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )

# Some really basic system things; not very portable
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DHAVE_DARWIN)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DHAVE_LINUX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
