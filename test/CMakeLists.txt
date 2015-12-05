#
# Copyright 2015 by Idiap Research Institute, http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, December 2015
#

# Testing
#
# The tests get built here, but they are run from a different cmake
# file.  This lets us compare output with a reference output rather
# than just test that something runs.

set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)
add_definitions(-DTEST_DIR="${TEST_DIR}")

add_executable(test-feature test-feature.cpp)
add_executable(testfile testfile.c)
target_link_libraries(test-feature static-lib ${TARGET_LIBS})
add_test(
  NAME feature
  COMMAND ${CMAKE_COMMAND} -D TEST_DIR=${TEST_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-feature.cmake
  )

add_executable(test-fft test-fft.cpp)
target_link_libraries(test-fft static-lib ${TARGET_LIBS})
add_test(
  NAME fft
  COMMAND ${CMAKE_COMMAND} -D TEST_DIR=${TEST_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-fft.cmake
  )

add_executable(test-tokenise test-tokenise.cpp)
target_link_libraries(test-tokenise static-lib ${TARGET_LIBS})
add_test(
  NAME tokenise
  COMMAND ${CMAKE_COMMAND} -D TEST_DIR=${TEST_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-tokenise.cmake
  )
