#
# Copyright 2007,2008 by IDIAP Research Institute
#                        http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

bin_PROGRAMS = \
	tracter htktopixmap

noinst_PROGRAMS = \
	socketserver socketclient \
	creature testfile fft parser

lib_LTLIBRARIES = libtracter.la

TRACTER_SOURCES = \
	SocketTee.cpp \
	ScreenSink.cpp \
	LPCepstrum.cpp \
	FileSink.cpp \
	VADGate.cpp \
	VADStateMachine.cpp \
	ModulationVAD.cpp \
	Energy.cpp \
	SlidingDFT.cpp \
	TracterObject.cpp \
	ASRFactory.cpp \
	LowEnergyEnvelope.cpp \
	Tokenise.cpp \
	Histogram.cpp \
	Laguerre.cpp \
	Variance.cpp \
	Divide.cpp \
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
	Concatenate.cpp \
	Periodogram.cpp \
	ZeroFilter.cpp \
	MelFilter.cpp \
	Cepstrum.cpp \
	Mean.cpp \
	Noise.cpp \
	Subtract.cpp \
	SpectralSubtract.cpp \
	Pixmap.cpp

pkginclude_HEADERS = $(TRACTER_SOURCES:.cpp=.h) \
	Fourier.h \
	ArraySink.h \
	CachedPlugin.h \
	FileSource.h \
	Plugin.h \
	Sink.h \
	Source.h \
	UnaryPlugin.h \
	UnarySink.h

libtracter_la_SOURCES = $(TRACTER_SOURCES)

# Blank, but added to later
nodist_libtracter_la_SOURCES=
libtracter_la_CPPFLAGS=
libtracter_la_CFLAGS= 

if WITH_ALSA
TRACTER_SOURCES += ALSASource.cpp
noinst_PROGRAMS += alsafile
endif

if WITH_HTK
TRACTER_SOURCES += HCopyWrapper.cpp
libtracter_la_CPPFLAGS += @HTK_INCLUDES@
noinst_PROGRAMS += hcopytester
endif

if WITH_KISS_FFT
libtracter_la_SOURCES += \
	FourierKiss.cpp
nodist_libtracter_la_SOURCES += \
	@KISS_FFT_DIR@/tools/kiss_fftr.c \
	@KISS_FFT_DIR@/kiss_fft.c
libtracter_la_CPPFLAGS += -I@KISS_FFT_DIR@ -I@KISS_FFT_DIR@/tools
endif

libtracter_la_LIBADD = @ALSA_LIBS@ @HTK_LIBS@ -lpthread

parser_SOURCES = parser.cpp
parser_LDADD = libtracter.la

fft_SOURCES = fft.cpp
fft_LDADD = libtracter.la

testfile_SOURCES = testfile.c

tracter_SOURCES = tracter.cpp
tracter_LDADD = libtracter.la

creature_SOURCES = creature.cpp
creature_LDADD = libtracter.la

socketclient_SOURCES = socketclient.cpp
socketclient_LDADD = libtracter.la

socketserver_SOURCES = socketserver.cpp
socketserver_LDADD = libtracter.la

alsafile_SOURCES = alsafile.cpp
alsafile_LDADD = libtracter.la

htktopixmap_SOURCES = htktopixmap.cpp
htktopixmap_LDADD = libtracter.la

hcopytester_SOURCES = hcopytester.cpp
hcopytester_CPPFLAGS = @HTK_INCLUDES@
hcopytester_LDADD = libtracter.la
