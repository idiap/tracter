#
# Find KISSFFT
#
set(KISSFFT_DIR $ENV{KISSFFT_DIR}
  CACHE FILEPATH "Path to Kiss FFT directory"
  )

if ("${KISSFFT_DIR}" MATCHES "")
  set(KISSFFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kissfft")
endif ("${KISSFFT_DIR}" MATCHES "")

if (EXISTS "${KISSFFT_DIR}")
  message(STATUS "Using Kiss FFT dir: ${KISSFFT_DIR}")
  set(KISSFFT_FOUND 1)
else (EXISTS "${KISSFFT_DIR}")
  if (KissFFT_FIND_REQUIRED)
    message(FATAL_ERROR "Kiss FFT not found")
  endif (KissFFT_FIND_REQUIRED)
  set(KISSFFT_FOUND 0)
endif (EXISTS "${KISSFFT_DIR}")
