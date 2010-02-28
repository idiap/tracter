#
# Find LibResample
#
set(LIBRESAMPLE_DIR $ENV{LIBRESAMPLE_DIR}
  CACHE FILEPATH "Path to libresample directory"
  )

if (EXISTS "${LIBRESAMPLE_DIR}")
  message(STATUS "Using libresample dir: ${LIBRESAMPLE_DIR}")
  set(LIBRESAMPLE_FOUND 1)
else (EXISTS "${LIBRESAMPLE_DIR}")
  message(STATUS "libresample not found")
  set(LIBRESAMPLE_FOUND 0)
endif (EXISTS "${LIBRESAMPLE_DIR}")
