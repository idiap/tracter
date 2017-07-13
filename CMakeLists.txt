#
# CMake file for Tracter
#
# Phil Garner
# March 2009
#

# Idiap's Debian; essentially the oldest version that I use
cmake_minimum_required(VERSION 3.6)

# Top level project information
project(tracter)
set(VERSION 2.0a)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# This is required for ssp headers
set(CMAKE_CXX_FLAGS "-std=c++11")

# Find boost.  The commented part gets rid of the version number; this can help
# with nonhomegeneous clusters.
find_package(Boost COMPONENTS system filesystem REQUIRED)
# string(REGEX REPLACE "'.'so.*" ".so" Boost_LIBRARIES "${Boost_LIBRARIES}")
message(STATUS "Boost lib is ${Boost_LIBRARIES}")

find_package(LibUBE REQUIRED)
include_directories(${LIBUBE_INCLUDE_DIRS})

add_subdirectory(bin)
add_subdirectory(tracter)

# Executables
add_executable(extracter extracter.cpp)
target_link_libraries(extracter static-lib)
set(INSTALL_TARGETS
  extracter
)

find_package(SndFile)
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
