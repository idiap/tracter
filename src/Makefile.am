#
# Copyright 2007,2008 by IDIAP Research Institute
#                        http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

bin_PROGRAMS = \
	socketserver socketclient \
	creature testfile tracter noise fft

lib_LIBRARIES = libtracter.a

libtracter_a_SOURCES = \
	MAPNoise.cpp \
	Resample.cpp \
	FilePath.cpp \
	SocketSink.cpp \
	SocketSource.cpp \
	ComplexPeriodogram.cpp \
	ComplexSample.cpp \
	ByteOrder.cpp \
	LNASource.cpp \
	PluginObject.cpp \
	Normalise.cpp \
	MMap.cpp \
	HTKSource.cpp \
	HTKSink.cpp \
	Delta.cpp \
	ALSASource.cpp \
	Concatenate.cpp \
	Periodogram.cpp \
	ZeroFilter.cpp \
	MelFilter.cpp \
	Cepstrum.cpp \
	Mean.cpp \
	Noise.cpp \
	Subtract.cpp \
	SpectralSubtract.cpp \
	PLP.cpp \
	Pixmap.cpp

include_HEADERS = $(libtracter_a_SOURCES:.cpp=.h) \
	Fourier.h \
	ArraySink.h \
	CachedPlugin.h \
	FileSource.h \
	Plugin.h \
	Sink.h \
	Source.h \
	UnaryPlugin.h \
	UnarySink.h


# Blank, but added to later
dist_libtracter_a_SOURCES=
nodist_libtracter_a_SOURCES=
libtracter_a_CPPFLAGS=
libtracter_a_CFLAGS= 

# nodist has the side effect of not building FourierXXXX.h with .cpp=.h
if WITH_KISS_FFT
nodist_libtracter_a_SOURCES += \
	FourierKiss.cpp \
	@KISS_FFT_DIR@/tools/kiss_fftr.c \
	@KISS_FFT_DIR@/kiss_fft.c
dist_libtracter_a_SOURCES += FourierKiss.cpp
libtracter_a_CPPFLAGS += -I@KISS_FFT_DIR@ -I@KISS_FFT_DIR@/tools
FFT_LIBS =
endif

if WITH_FFTW3F
nodist_libtracter_a_SOURCES += FourierFFTW.cpp
dist_libtracter_a_SOURCES += FourierFFTW.cpp
libtracter_a_CFLAGS += @FFTW3F_CFLAGS@
FFT_LIBS = @FFTW3F_LIBS@
endif

noise_SOURCES = noise.cpp
noise_LDADD = libtracter.a

fft_SOURCES = fft.cpp
fft_LDADD = libtracter.a $(FFT_LIBS)

testfile_SOURCES = testfile.c

tracter_SOURCES = tracter.cpp
tracter_LDADD = libtracter.a $(FFT_LIBS)

creature_SOURCES = creature.cpp
creature_LDADD = libtracter.a @SAMPLERATE_LIBS@ $(FFT_LIBS)

socketclient_SOURCES = socketclient.cpp
socketclient_LDADD = libtracter.a

socketserver_SOURCES = socketserver.cpp
socketserver_LDADD = libtracter.a
