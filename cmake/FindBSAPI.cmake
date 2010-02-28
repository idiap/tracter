#
# Find BSAPI
#
# Doesn't actually look very hard.  If there's a directory with the
# right name, we're there.
#
set(BSAPI_DIR $ENV{BSAPI_DIR}
  CACHE FILEPATH "Path to BSAPI distribution"
  )

if (EXISTS "${BSAPI_DIR}")
  message(STATUS "Using BSAPI dir: ${BSAPI_DIR}")
  set(BSAPI_FOUND 1)
  set(BSAPI_INCLUDE_DIRS
    ${BSAPI_DIR}
    )
  set(BSAPI_LIBRARIES
    ${BSAPI_DIR}/bsapi.so
    )
else (EXISTS "${BSAPI_DIR}")
  message(STATUS "BSAPI not found")
  set(BSAPI_FOUND 0)
endif (EXISTS "${BSAPI_DIR}")
