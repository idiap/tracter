/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "HTKSource.h"
#include "Pixmap.h"
#include "ArraySink.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <htkfile>\n", argv[0]);
        return 1;
    }

    HTKSource* source = new HTKSource();
    Pixmap* pixmap = new Pixmap(source);
    ArraySink<float> sink(pixmap);
    source->Open(argv[1]);
    sink.Reset();

    float* frame = 0;
    int index = 0;
    while(sink.GetArray(frame, index++)) {}
    return 0;
}
