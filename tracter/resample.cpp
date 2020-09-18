/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "Resample.h"
#include "Normalise.h"
#ifdef HAVE_SNDFILE
# include "SndFileSource.h"
#endif
#include "FileSource.h"
#include "FileSink.h"
#include "Frame.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    printf("Resample to file\n");
    if (argc != 3)
    {
        printf("Usage: %s infile outfile.raw\n", argv[0]);
        return 1;
    }

#ifdef HAVE_SNDFILE
    SndFileSource* source = new SndFileSource();
    Resample* r = new Resample(source);
#else
    FileSource<short>* source = new FileSource<short>();
    Normalise* n = new Normalise(source);
    Resample* r = new Resample(n);
#endif
    Frame* f = new frame(r);
    FileSink sink(f);

    source->open(argv[1]);
    sink.reset();
    sink.open(argv[2]);

    printf("Done\n");
    return 0;
}
