include_directories(${CMAKE_CURRENT_SOURCE_DIR})
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
  Cepstrum.cpp
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
  FilePath.cpp
  FileSink.cpp
  Frame.cpp
  Histogram.cpp
  HTKLib.cpp
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
  Periodogram.cpp
  Pixmap.cpp
  SNRSpectrum.cpp
  ScreenSink.cpp
  Select.cpp
  Signal.cpp
  SocketSink.cpp
  SocketSource.cpp
  SocketTee.cpp
  SpeakerIDSocketSource.cpp
  SpectralSubtract.cpp
  StreamSocketSource.cpp
  Subtract.cpp
  Tokenise.cpp
  TracterObject.cpp
  TransverseFilter.cpp
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

# Kiss is compulsory for now
# The library is not supplied built, so we add in the sources explicitly.
if(KISSFFT_FOUND)
  list(APPEND SOURCES
    FourierKiss.cpp
    ${KISSFFT_DIR}/tools/kiss_fftr.c
    ${KISSFFT_DIR}/kiss_fft.c
    )
  include_directories(${KISSFFT_DIR} ${KISSFFT_DIR}/tools)
endif(KISSFFT_FOUND)

# HTK is optional
if(HTK_FOUND)
  list(APPEND SOURCES HCopyWrapper.cpp HTKLibSource.cpp)
  list(APPEND HEADERS HCopyWrapper.h   HTKLibSource.h  )
  add_definitions(-DHAVE_HTKLIB)
  include_directories(${HTK_INCLUDE_DIRS})
endif(HTK_FOUND)

# Torch3 is optional
if(TORCH3_FOUND)
  list(APPEND SOURCES
    BlasLinear.cpp
    SKMLP.cpp
    MLP.cpp
    MLPVAD.cpp
    )
  list(APPEND HEADERS
    BlasLinear.h
    SKMLP.h
    MLP.h
    MLPVAD.h
    )
  add_definitions(-DHAVE_TORCH3)
  include_directories(${TORCH3_INCLUDE_DIRS})
  link_directories(${TORCH3_LIBRARY_DIRS})
endif(TORCH3_FOUND)

# BSAPI comes as a shared library
if(BSAPI_FOUND)
  list(APPEND SOURCES
    BSAPITransform.cpp
    BSAPIFrontEnd.cpp
    BSAPIFilterBank.cpp
    BSAPIFastVTLN.cpp
    )
  list(APPEND HEADERS
    BSAPITransform.h
    BSAPIFrontEnd.h
    BSAPIFilterBank.h
    BSAPIFastVTLN.h
    )
  add_definitions(-DHAVE_BSAPI)
  include_directories(${BSAPI_INCLUDE_DIRS})
endif(BSAPI_FOUND)

# ALSA is a system thng, depends on whether pkgconfig found it
if(ALSA_FOUND)
  list(APPEND SOURCES ALSASource.cpp)
  list(APPEND HEADERS ALSASource.h  )
  add_definitions(-DHAVE_ALSA)
  include_directories(${ALSA_INCLUDE_DIRS})
  list(APPEND PKGCONFIG_REQUIRES alsa)
endif(ALSA_FOUND)

# SndFile pretty much like ALSA
if(SNDFILE_FOUND)
  list(APPEND SOURCES SndFileSource.cpp)
  list(APPEND HEADERS SndFileSource.h  )
  add_definitions(-DHAVE_SNDFILE)
  include_directories(${SNDFILE_INCLUDE_DIRS})
  list(APPEND PKGCONFIG_REQUIRES sndfile)
endif(SNDFILE_FOUND)

# libresample
if(LIBRESAMPLE_FOUND) 
  list(APPEND SOURCES ResampleLRS.cpp)
  add_definitions(-DHAVE_RESAMPLE)
  include_directories(${LIBRESAMPLE_INCLUDE_DIRS})
endif(LIBRESAMPLE_FOUND)

# RtAudio
if(RTAUDIO_FOUND)
  list(APPEND SOURCES RtAudioSource.cpp)
  list(APPEND HEADERS RtAudioSource.h  )
  add_definitions(-DHAVE_RTAUDIO)
  include_directories(${RTAUDIO_INCLUDE_DIRS})
endif(RTAUDIO_FOUND)

# SPTK
if(SPTK_FOUND)
  list(APPEND SOURCES MCep.cpp)
  list(APPEND HEADERS MCep.h  )
  add_definitions(-DHAVE_SPTK)
  include_directories(${SPTK_INCLUDE_DIRS})
endif(SPTK_FOUND)

# A little wordy, but allows static lib also with the same syntax
add_library(shared-lib SHARED ${SOURCES})
set_target_properties(shared-lib
  PROPERTIES OUTPUT_NAME   "tracter"
             PUBLIC_HEADER "${HEADERS}"
  )

target_link_libraries(shared-lib
  ${TORCH3_LIBRARIES}
  ${HTK_LIBRARIES}
  ${BSAPI_LIBRARIES}
  ${ALSA_LIBRARIES}
  ${SNDFILE_LIBRARIES}
  ${SPTK_LIBRARIES}
  ${RTAUDIO_LIBRARIES}
  ${LIBRESAMPLE_LIBRARIES}
  )

add_executable(extracter extracter.cpp)
add_executable(recorder recorder.cpp)

add_executable(socketserver socketserver.cpp)
add_executable(socketclient socketclient.cpp)

target_link_libraries(extracter shared-lib)
target_link_libraries(recorder  shared-lib)
target_link_libraries(socketserver shared-lib)
target_link_libraries(socketclient shared-lib)

# CMake install line
install(
  TARGETS extracter recorder shared-lib
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/tracter
  )

# This feels like a hack.
string(REGEX REPLACE ";" " -I" TORCH3_CFLAGS "-I${TORCH3_INCLUDE_DIRS}")
string(REGEX REPLACE ";" "," TRACTER_REQUIRES "${PKGCONFIG_REQUIRES}")

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
