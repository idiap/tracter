/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdio.h>
#include "ALSASource.h"
#include "Normalise.h"
#include "FileSink.h"


int main(int argc, char** argv)
{
    printf("ALSA to file\n");

    Tracter::sShowConfig = true;
    ALSASource* source = new ALSASource();
    Normalise* n = new Normalise(source);
    FileSink sink(n);
    source->Open("plughw:1");
    sink.Reset();
    sink.Open("alsa.raw");

    printf("Done\n");
    return 0;
}
