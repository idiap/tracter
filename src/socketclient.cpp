/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "SocketSource.h"
#include "ArraySink.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    printf("Socket client\n");

    SocketSource* ss = new SocketSource;
    ArraySink<float> as(ss);
    ss->Open("localhost");
    as.Reset();
    float* frame;
    int index = 0;
    while(as.GetArray(frame, index++) && index < 1000)
    {
        printf("%.3f\n", frame[0] * 32768);
    }

    printf("Done\n");
    return 0;
}
