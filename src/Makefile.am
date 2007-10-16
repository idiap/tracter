#
# Copyright 2007 by IDIAP Research Institute
#                   http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

bin_PROGRAMS = creature testfile
lib_LIBRARIES = libtracter.a

libtracter_a_SOURCES = \
	PluginObject.cpp \
	Normalise.cpp \
	MMap.cpp \
	HTKFile.cpp \
	Delta.cpp \
	ALSASource.cpp \
	Feature.cpp \
	Periodogram.cpp \
	ZeroFilter.cpp \
	MelFilter.cpp \
	Cepstrum.cpp \
	Mean.cpp

testfile_SOURCES = testfile.c
creature_SOURCES = creature.cpp
creature_LDADD = libtracter.a
