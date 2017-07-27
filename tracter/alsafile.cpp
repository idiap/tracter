/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#ifdef HAVE_RTAUDIO
# include "RtAudioSource.h"
#else
# include "ALSASource.h"
# include "Normalise.h"
#endif
#include "Resample.h"
#include "FileSink.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    printf("ALSA to file\n");
    if (argc != 2)
    {
        printf("Usage: %s <device>\n", argv[0]);
        return 1;
    }

#ifdef HAVE_RTAUDIO
    RtAudioSource* source = new RtAudioSource();
    FileSink sink(source);
#else
    ALSASource* source = new ALSASource();
    Normalise* n = new Normalise(source);
    //Resample* r = new Resample(n);
    FileSink sink(n);
#endif
    source->open(argv[1]);
    sink.reset();
    sink.open("alsa.raw");

    printf("Done\n");
    return 0;
}
