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
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
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
find_package(LibSSP)

# This is required for ssp headers
set(CMAKE_CXX_FLAGS "-std=c++11")

add_subdirectory(bin)
add_subdirectory(tracter)

# Executables
add_executable(extracter extracter.cpp)
target_link_libraries(extracter static-lib)
set(INSTALL_TARGETS
  extracter
)

if(SNDFILE_FOUND)
  # codec and recorder rely on SndFile
  add_executable(codec codec.cpp)
  add_executable(recorder recorder.cpp)
  target_link_libraries(codec static-lib)
  target_link_libraries(recorder  static-lib)
  list(APPEND INSTALL_TARGETS recorder codec)
endif(SNDFILE_FOUND)

install(
  TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION bin
  )

# Configure other files
configure_file(Doxyfile.in Doxyfile)

# Testing
enable_testing()
add_subdirectory(test)

# This one is for binary distributions
include(CPack)

# For source, it's *much* better to use git
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${VERSION})
add_custom_target(dist
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
    | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
