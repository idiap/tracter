CC  = gcc
CXX = g++
CPPFLAGS = -ansi -MD -Wall -g
LDLIBS = -lfftw3f -lasound -lstdc++

VPATH = src

OBJ = \
	PluginObject.o \
	Normalise.o \
	MMap.o \
	HTKFile.o \
	Delta.o \
	ALSASource.o \
	Feature.o \
	Periodogram.o \
	ZeroFilter.o \
	MelFilter.o \
	Cepstrum.o \
	Mean.o \

EXE = creature testfile

DEP = $(OBJ:.o=.d) creature.d testfile.d

all: $(EXE)

testfile: testfile.c

clean:
	rm -f $(OBJ) $(DEP) \
		libfeatures.a creature.o creature testfile testfile.dat

libfeatures.a: libfeatures.a($(OBJ))

creature: $(OBJ) creature.o

-include $(DEP)
