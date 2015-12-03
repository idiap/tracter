/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "HTKSource.h"
#include "Pixmap.h"
#include "FrameSink.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <htkfile>\n", argv[0]);
        return 1;
    }

    HTKSource* source = new HTKSource();
    Pixmap* pixmap = new Pixmap(source);
    FrameSink<float> sink(pixmap);
    source->Open(argv[1]);
    sink.Reset();

    int index = 0;
    while(sink.Read(index++)) {}
    return 0;
}
