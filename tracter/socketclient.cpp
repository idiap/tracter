/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "StreamSocketSource.h"
#include "FrameSink.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    printf("Socket client\n");

    StreamSocketSource* ss = new StreamSocketSource;
    FrameSink<float> as(ss);
    ss->open("localhost");
    as.reset();
    int index = 0;
    while(const float* frame = as.Read(index))
    {
        printf("Val: %.3f ", frame[0] * 32768);
        printf("Time: %lld = %.3f s\n",
               as.timeStamp(index), as.seconds(index));
        index++;
        if (index >= 1000)
            break;
    }

    printf("Done\n");
    return 0;
}
