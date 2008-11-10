/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include <sys/time.h>
#include <time.h>

#include "FileSource.h"
#include "Normalise.h"
#include "SocketSink.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    /* Set time */
    struct timeval tv;
    if (gettimeofday(&tv, 0))
        exit(1);
    TimeType time = (TimeType)tv.tv_sec * 1e9;
    time += (TimeType)tv.tv_usec * 1e3;
    printf("Time is %lld\n", time);

    /* Processing chain */
    FileSource<short>* fs = new FileSource<short>;
    fs->SetTime(time);
    Normalise* n = new Normalise(fs);
    SocketSink ss(n);

    /* Run */
    fs->Open("testfile.dat");
    ss.Pull();

    printf("Done\n");
    return 0;
}
