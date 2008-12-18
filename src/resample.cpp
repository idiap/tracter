/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "Resample.h"
#include "Normalise.h"
#include "SndFileSource.h"
#include "FileSource.h"
#include "FileSink.h"
#include "Frame.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    printf("Resample to file\n");
    if (argc != 3)
    {
        printf("Usage: %s infile.raw outfile.raw\n", argv[0]);
        return 1;
    }

#if 0
    FileSource<short>* source = new FileSource<short>();
    Normalise* n = new Normalise(source);
    Resample* r = new Resample(n);
#else
    SndFileSource* source = new SndFileSource();
    Resample* r = new Resample(source);
#endif
    Frame* f = new Frame(r);
    FileSink sink(f);

    source->Open(argv[1]);
    sink.Reset();
    sink.Open(argv[2]);

    printf("Done\n");
    return 0;
}
