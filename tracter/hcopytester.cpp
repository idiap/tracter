/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdlib.h>
#include <stdio.h>

#include "FileSource.h"
#include "ScreenSink.h"
#include "HCopyWrapper.h"
#include "Normalise.h"

using namespace Tracter;

void Usage()
{
    puts(
        "Usage: tracter [infile]\n"
    );
}

/**
 * Tracter executable.
 */
int main(int argc, char** argv)
{
    const char* file = 0;
    int fileCount = 0;
    for (int i=1; i<argc; i++)
    {
        /* Unqualified arguments are the input and output files */
        if (argv[i][0] != '-')
        {
            if (fileCount++ < 1)
                file = argv[i];
            else
            {
                printf("Too many unqualified arguments\n");
                exit(EXIT_FAILURE);
            }
            continue;
        }
    }

#ifdef HCOPY_FLOAT
    FileSource<short>* source = new FileSource<short>();
    Normalise* p = new Normalise(source);
    HCopyWrapper* f = new HCopyWrapper( p );
#else
    FileSource<short>* source = new FileSource<short>();
    HCopyWrapper* f = new HCopyWrapper( source );
#endif
    ScreenSink sink(f);
    fflush(0);

    if (fileCount < 1)
    {
        printf("Not enough files defined\n");
        exit(EXIT_FAILURE);
    }
    try
    {
        source->Open(file);
        sink.Open();
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "Caught exception: %s\n", e.what());
        return 1;
    }
    catch(...)
    {
        fprintf(stderr, "Caught unknown exception\n");
        return 1;
    }

    return 0;
}
