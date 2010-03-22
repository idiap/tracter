#
# Find Rtaudio
#
set(RTAUDIO_DIR $ENV{RTAUDIO_DIR}
  CACHE FILEPATH "Path to RtAudio directory"
  )

# Use the static library so it gets incorporated in the built library
# If RtAudio is "package" this is less of an issue
if (EXISTS "${RTAUDIO_DIR}")
  message(STATUS "Using RtAudio dir: ${RTAUDIO_DIR}")
  set(RTAUDIO_FOUND 1)
  set(RTAUDIO_INCLUDE_DIRS
    ${RTAUDIO_DIR}
    )
  set(RTAUDIO_LIBRARIES
    ${RTAUDIO_DIR}/librtaudio.a
    )  
else (EXISTS "${RTAUDIO_DIR}")
  message(STATUS "RtAudio not found")
  set(RTAUDIO_FOUND 0)
endif (EXISTS "${RTAUDIO_DIR}")
