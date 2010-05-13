/*
 * Copyright 2010 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "LNASource.h"
#include "ScreenSink.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    printf("LNA dumper\n");
    if (argc != 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    LNASource* source = new LNASource();
    ScreenSink sink(source);

    source->Open(argv[1]);
    sink.Reset();
    sink.Open();

    printf("Done\n");
    return 0;
}
