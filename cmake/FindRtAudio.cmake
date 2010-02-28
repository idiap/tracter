#
# Find Rtaudio
#
set(RTAUDIO_DIR $ENV{RTAUDIO_DIR}
  CACHE FILEPATH "Path to RtAudio directory"
  )

if (EXISTS "${RTAUDIO_DIR}")
  message(STATUS "Using RtAudio dir: ${RTAUDIO_DIR}")
  set(RTAUDIO_FOUND 1)
else (EXISTS "${RTAUDIO_DIR}")
  message(STATUS "RtAudio not found")
  set(RTAUDIO_FOUND 0)
endif (EXISTS "${RTAUDIO_DIR}")
