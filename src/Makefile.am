#
# Copyright 2007,2008 by IDIAP Research Institute
#                        http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

bin_PROGRAMS = \
	extracter htktopixmap recorder

noinst_PROGRAMS = \
	socketserver socketclient \
	creature testfile fft parser

lib_LTLIBRARIES = libtracter.la

libtracter_la_LDFLAGS = -version-info 1:0:0

TRACTER_SOURCES = \
	ViterbiVAD.cpp \
	ViterbiVADGate.cpp \
	HTKLib.cpp \
	Select.cpp \
	NoiseVAD.cpp \
	Modulation.cpp \
	Window.cpp \
	Extract.cpp \
	LinearTransform.cpp \
	Signal.cpp \
	Frame.cpp \
	SocketTee.cpp \
	Tokenise.cpp \
	LNASource.cpp \
	Pixmap.cpp \
	ScreenSink.cpp \
	LPCepstrum.cpp \
	FileSink.cpp \
	VADGate.cpp \
	VADStateMachine.cpp \
	Energy.cpp \
	TracterObject.cpp \
	ASRFactory.cpp \
	Variance.cpp \
	Divide.cpp \
	FilePath.cpp \
	SocketSink.cpp \
	SocketSource.cpp \
	StreamSocketSource.cpp \
	SpeakerIDSocketSource.cpp \
	ComplexPeriodogram.cpp \
	ComplexSample.cpp \
	ByteOrder.cpp \
	Component.cpp \
	Normalise.cpp \
	HTKSource.cpp \
	HTKSink.cpp \
	Delta.cpp \
	Concatenate.cpp \
	Periodogram.cpp \
	ZeroFilter.cpp \
	MelFilter.cpp \
	Cepstrum.cpp \
	Mean.cpp \
	Subtract.cpp \
	SpectralSubtract.cpp \
	Noise.cpp \
	LowEnergyEnvelope.cpp \
	Histogram.cpp \
	MMap.cpp

pkginclude_HEADERS = $(TRACTER_SOURCES:.cpp=.h) \
	GeometricNoise.h \
	Resample.h \
	Fourier.h \
	FrameSink.h \
	CachedComponent.h \
	FileSource.h \
	Sink.h \
	Source.h

libtracter_la_SOURCES = $(TRACTER_SOURCES)

# Blank, but added to later
nodist_libtracter_la_SOURCES=
libtracter_la_CPPFLAGS=
libtracter_la_CFLAGS= 

if WITH_TORCH3
TRACTER_SOURCES += \
	BlasLinear.cpp \
	SKMLP.cpp \
	MLP.cpp \
	MLPVAD.cpp
libtracter_la_CPPFLAGS += @TORCH3_INCLUDES@
endif

if WITH_BSAPI
TRACTER_SOURCES += \
	BSAPITransform.cpp \
	BSAPIFrontEnd.cpp \
	BSAPIFilterBank.cpp \
	BSAPIFastVTLN.cpp      
libtracter_la_CPPFLAGS += @BSAPI_INCLUDES@
endif

if WITH_ALSA
TRACTER_SOURCES += ALSASource.cpp
noinst_PROGRAMS += alsafile
endif

if WITH_SNDFILE
TRACTER_SOURCES += SndFileSource.cpp
libtracter_la_CPPFLAGS += @SNDFILE_CFLAGS@
noinst_PROGRAMS += snddump
endif

if WITH_HTK
TRACTER_SOURCES += HCopyWrapper.cpp
TRACTER_SOURCES += HTKLibSource.cpp
libtracter_la_CPPFLAGS += @HTK_INCLUDES@
noinst_PROGRAMS += hcopytester
endif

if WITH_SPTK
TRACTER_SOURCES += MCep.cpp
libtracter_la_CPPFLAGS += @SPTK_INCLUDES@
endif

if WITH_KISS_FFT
libtracter_la_SOURCES += \
	FourierKiss.cpp
nodist_libtracter_la_SOURCES += \
	@KISS_FFT_DIR@/tools/kiss_fftr.c \
	@KISS_FFT_DIR@/kiss_fft.c
libtracter_la_CPPFLAGS += -I@KISS_FFT_DIR@ -I@KISS_FFT_DIR@/tools
endif

if WITH_LIBRESAMPLE
libtracter_la_SOURCES += ResampleLRS.cpp
nodist_libtracter_la_SOURCES += \
	@LIBRESAMPLE_DIR@/src/resample.c \
	@LIBRESAMPLE_DIR@/src/resamplesubs.c \
	@LIBRESAMPLE_DIR@/src/filterkit.c
libtracter_la_CPPFLAGS += \
	-I@LIBRESAMPLE_DIR@/src \
	-I@LIBRESAMPLE_DIR@/include
noinst_PROGRAMS += resample
pkginclude_HEADERS += Resample.h
endif

if WITH_RTAUDIO
TRACTER_SOURCES += RtAudioSource.cpp
nodist_libtracter_la_SOURCES += \
	@RTAUDIO_DIR@/RtAudio.cpp
libtracter_la_CPPFLAGS += -I@RTAUDIO_DIR@ -D__LINUX_ALSA__
endif

libtracter_la_LIBADD = \
	@ALSA_LIBS@ \
	@SNDFILE_LIBS@ \
	@HTK_LIBS@ \
	@SPTK_LIBS@ \
	@BSAPI_LIBS@ \
	@TORCH3_LIBS@ \
	-lpthread

parser_SOURCES = parser.cpp
parser_LDADD = libtracter.la

fft_SOURCES = fft.cpp
fft_LDADD = libtracter.la

testfile_SOURCES = testfile.c

extracter_SOURCES = extracter.cpp
extracter_LDADD = libtracter.la

recorder_CPPFLAGS = @BSAPI_INCLUDES@ @TORCH3_INCLUDES@
recorder_SOURCES = recorder.cpp
recorder_LDADD = libtracter.la

creature_SOURCES = creature.cpp
creature_LDADD = libtracter.la

socketclient_SOURCES = socketclient.cpp
socketclient_LDADD = libtracter.la

socketserver_SOURCES = socketserver.cpp
socketserver_LDADD = libtracter.la

alsafile_SOURCES = alsafile.cpp
alsafile_LDADD = libtracter.la
alsafile_CPPFLAGS = -I@RTAUDIO_DIR@ -D__LINUX_ALSA__

htktopixmap_SOURCES = htktopixmap.cpp
htktopixmap_LDADD = libtracter.la

hcopytester_SOURCES = hcopytester.cpp
hcopytester_CPPFLAGS = @HTK_INCLUDES@
hcopytester_LDADD = libtracter.la

resample_SOURCES = resample.cpp
resample_LDADD = libtracter.la

snddump_SOURCES = snddump.cpp
snddump_CPPFLAGS = @SNDFILE_CFLAGS@
snddump_LDADD = libtracter.la
