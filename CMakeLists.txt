#
# CMake file for Tracter
#
# Phil Garner
# March 2009
#

# Version 2.4 might work, but it won't find static libraries with odd names
cmake_minimum_required(VERSION 2.6)
project(Tracter)
set(VERSION 0.6.0)

# CMake complains without this
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

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

configure_file(Doxyfile.in Doxyfile)
configure_file(tracter.pc.in tracter.pc)

add_subdirectory(src)

include(CPack)