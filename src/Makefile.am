#
# Copyright 2007 by IDIAP Research Institute
#                   http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

bin_PROGRAMS = socketserver socketclient creature testfile tracter
lib_LIBRARIES = libtracter.a

libtracter_a_SOURCES = \
	ConvertSampleRate.cpp \
	FileName.cpp \
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
	ArraySink.h \
	CachedPlugin.h \
	FileSource.h \
	Plugin.h \
	Sink.h \
	Source.h \
	UnaryPlugin.h \
	UnarySink.h

testfile_SOURCES = testfile.c

tracter_SOURCES = tracter.cpp
tracter_LDADD = libtracter.a @FFTW3F_LIBS@

creature_SOURCES = creature.cpp
creature_LDADD = libtracter.a @SAMPLERATE_LIBS@ @FFTW3F_LIBS@

socketclient_SOURCES = socketclient.cpp
socketclient_LDADD = libtracter.a

socketserver_SOURCES = socketserver.cpp
socketserver_LDADD = libtracter.a
