/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdio.h>
#include "SocketSource.h"
#include "ArraySink.h"


int main(int argc, char** argv)
{
    printf("Socket client\n");

    Tracter::sShowConfig = true;
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
