/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "SndFileSource.h"
#include "ScreenSink.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    SndFileSource* source = new SndFileSource();
    ScreenSink sink(source);

    source->open(argv[1]);
    sink.open();

    return 0;
}
