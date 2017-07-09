include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DPACKAGE_VERSION="${VERSION}")

# Some really basic system things; not very portable
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DHAVE_DARWIN)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DHAVE_LINUX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

find_path(FPU fpu_control.h)
if(${FPU} MATCHES "FPU-NOTFOUND")
  message(STATUS "fpu_control.h not found")
else()
  message(STATUS "fpu_control.h found")
  add_definitions(-DHAVE_FPU_CONTROL)
endif()

# Basic all-the-time sources
set(SOURCES
  ASRFactory.cpp
  ByteOrder.cpp
  BoolToFloat.cpp
  Cepstrum.cpp
  Comparator.cpp
  ComplexPeriodogram.cpp
  ComplexSample.cpp
  Component.cpp
  Concatenate.cpp
  CosineTransform.cpp
  Delta.cpp
  Divide.cpp
  Energy.cpp
  EnergyNorm.cpp
  Extract.cpp
  FPE.cpp
  FilePath.cpp
  FileSink.cpp
  FourierTransform.cpp
  Frame.cpp
  Gate.cpp
  Histogram.cpp
  HTKSink.cpp
  HTKSource.cpp
  LinearTransform.cpp
  LNASource.cpp
  Log.cpp
  LowEnergyEnvelope.cpp
  LPCepstrum.cpp
  Mean.cpp
  MelFilter.cpp
  Minima.cpp
  MMap.cpp
  Modulation.cpp
  Noise.cpp
  NoiseVAD.cpp
  Normalise.cpp
  Object.cpp
  OverlapAdd.cpp
  Periodogram.cpp
  Pixmap.cpp
  SNRSpectrum.cpp
  ScreenSink.cpp
  Select.cpp
  SocketSink.cpp
  SocketSource.cpp
  SocketTee.cpp
  SpectralSubtract.cpp
  StreamSocketSource.cpp
  Subtract.cpp
  TimedLatch.cpp
  Tokenise.cpp
  TransverseFilter.cpp
  Unframe.cpp
  VADGate.cpp
  VADStateMachine.cpp
  Variance.cpp
  ViterbiVAD.cpp
  ViterbiVADGate.cpp
  Window.cpp
  ZeroFilter.cpp
  )

# Prime the header list with the cpp sources, then add the lone ones
string(REGEX REPLACE ".cpp" ".h" HEADERS "${SOURCES}")
list(APPEND HEADERS
  CachedComponent.h
  FileSource.h
  Fourier.h
  FrameSink.h
  GeometricNoise.h
  Resample.h
  Sink.h
  Source.h
  )

# Things to install
set(INSTALL_TARGETS
  static-lib
)

# A little wordy, but allows static lib also with the same syntax
set(TARGET_LIBS
  ${Boost_LIBRARIES}
)

# Find other packages
#
# To build *totally* static, comment out ALSA, SndFile, PulseAudio and
# RtAudio.  Pulse on Ubuntu doesn't have static libraries; the others
# use dynamic loading (maybe...)

# Kiss is compulsory for now
# The library is not supplied built, so we add in the sources explicitly.
set(KISSFFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../kissfft")
list(APPEND SOURCES
  FourierKiss.cpp
  ${KISSFFT_DIR}/tools/kiss_fftr.c
  ${KISSFFT_DIR}/kiss_fft.c
  )
include_directories(${KISSFFT_DIR} ${KISSFFT_DIR}/tools)

find_package(LibUBE REQUIRED)
include_directories(${LIBUBE_INCLUDE_DIRS})
list(APPEND TARGET_LIBS ${LIBUBE_LIBRARIES})

# ALSA is a system thing, depends on whether pkgconfig found it
find_package(ALSA)
if(ALSA_FOUND)
  list(APPEND SOURCES ALSASource.cpp)
  list(APPEND HEADERS ALSASource.h  )
  add_definitions(-DHAVE_ALSA)
  include_directories(${ALSA_INCLUDE_DIRS})
  list(APPEND TARGET_LIBS ${ALSA_LIBRARIES})
  list(APPEND PKGCONFIG_REQUIRES alsa)
endif(ALSA_FOUND)

# SndFile pretty much like ALSA
find_package(SndFile)
if(SNDFILE_FOUND)
  list(APPEND SOURCES SndFileSource.cpp SndFileSink.cpp)
  list(APPEND HEADERS SndFileSource.h   SndFileSink.h  )
  add_definitions(-DHAVE_SNDFILE)
  include_directories(${SNDFILE_INCLUDE_DIRS})
  list(APPEND TARGET_LIBS ${SNDFILE_LIBRARIES})
  list(APPEND PKGCONFIG_REQUIRES sndfile)
endif(SNDFILE_FOUND)

# libresample
find_package(LibResample)
if(LIBRESAMPLE_FOUND) 
  list(APPEND SOURCES ResampleLRS.cpp)
  add_definitions(-DHAVE_RESAMPLE)
  include_directories(${LIBRESAMPLE_INCLUDE_DIRS})
  list(APPEND TARGET_LIBS ${LIBRESAMPLE_LIBRARIES})
endif(LIBRESAMPLE_FOUND)

# RtAudio
find_package(RtAudio)
if(RTAUDIO_FOUND)
  list(APPEND SOURCES RtAudioSource.cpp)
  list(APPEND HEADERS RtAudioSource.h  )
  add_definitions(-DHAVE_RTAUDIO)
  include_directories(${RTAUDIO_INCLUDE_DIRS})
  list(APPEND TARGET_LIBS ${RTAUDIO_LIBRARIES})
endif(RTAUDIO_FOUND)

# SPTK
find_package(SPTK)
if(SPTK_FOUND)
  list(APPEND SOURCES MCep.cpp)
  list(APPEND HEADERS MCep.h  )
  add_definitions(-DHAVE_SPTK)
  include_directories(${SPTK_INCLUDE_DIRS})
  list(APPEND TARGET_LIBS ${SPTK_LIBRARIES})
endif(SPTK_FOUND)

# PulseAudio is another pkgconfig one
find_package(PulseAudio)
if(PULSEAUDIO_FOUND)
  list(APPEND SOURCES PulseAudioSource.cpp) # PulseAudioSink.cpp)
  list(APPEND HEADERS PulseAudioSource.h)   # PulseAudioSink.h  )
  add_definitions(-DHAVE_PULSEAUDIO)
  include_directories(${PULSEAUDIO_INCLUDE_DIRS})
  list(APPEND TARGET_LIBS ${PULSEAUDIO_LIBRARIES})
  list(APPEND PKGCONFIG_REQUIRES libpulse-simple)
endif(PULSEAUDIO_FOUND)

# LibSSP (why the if?)
if (NOT LIBSSP_FOUND)
  find_package(LibSSP)
endif (NOT LIBSSP_FOUND)
if(LIBSSP_FOUND)
  list(APPEND SOURCES CochlearFilter.cpp CochlearFrame.cpp)
  list(APPEND HEADERS CochlearFilter.h   CochlearFrame.h  )
  add_definitions(-DHAVE_LIBSSP)
  include_directories(${LIBSSP_INCLUDE_DIRS})
  list(APPEND TARGET_LIBS ${LIBSSP_LIBRARIES})
endif(LIBSSP_FOUND)

# Static library
add_library(static-lib STATIC ${SOURCES})
set_target_properties(static-lib
  PROPERTIES OUTPUT_NAME   "tracter"
             PUBLIC_HEADER "${HEADERS}"
  )
target_link_libraries(static-lib ${TARGET_LIBS})

# Shared library
set(USE_SHARED $ENV{USE_SHARED}
  CACHE BOOLEAN "Whether to build shared library"
  )
if (USE_SHARED)
  add_library(shared-lib SHARED ${SOURCES})
  set_target_properties(shared-lib
    PROPERTIES OUTPUT_NAME   "tracter"
    PUBLIC_HEADER "${HEADERS}"
    )
  target_link_libraries(shared-lib ${TARGET_LIBS})
  list(APPEND INSTALL_TARGETS shared-lib)
endif (USE_SHARED)

add_executable(socketserver socketserver.cpp)
add_executable(socketclient socketclient.cpp)
add_executable(lnadump lnadump.cpp)

# These link static for the time being.  Could be changed.
target_link_libraries(socketserver static-lib)
target_link_libraries(socketclient static-lib)
target_link_libraries(lnadump static-lib)

# CMake install line
install(
  TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/tracter
  )

# This feels like a hack.
#string(REGEX REPLACE ";" "," TRACTER_REQUIRES "${PKGCONFIG_REQUIRES}")

# pkgconfig install lines
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(includedir "\${prefix}/include/tracter")
configure_file(tracter.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tracter.pc)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/tracter.pc
  DESTINATION lib/pkgconfig
  )
