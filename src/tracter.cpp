/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdlib.h>
#include <stdio.h>

#include "HTKSink.h"
#include "FilePath.h"
#include "ASRFactory.h"

using namespace Tracter;

void Usage()
{
    puts(
        "Usage: tracter [options] [infile outfile | -f file-list]\n"
        "Options:\n"
        "-f list Read input and output files from list\n"
        "Anything else prints this information\n"
        "Set environment variable Tracter_shConfig to 1 for more options\n"
    );
}

/**
 * Tracter executable.
 */
int main(int argc, char** argv)
{
    const char* fileList = 0;
    const char* file[2] = {0, 0};
    int fileCount = 0;
    for (int i=1; i<argc; i++)
    {
        /* Unqualified arguments are the input and output files */
        if (argv[i][0] != '-')
        {
            if (fileCount < 2)
                file[fileCount++] = argv[i];
            else
            {
                printf("Too many unqualified arguments\n");
                exit(EXIT_FAILURE);
            }
            continue;
        }

        /* Arguments beginning with a '-' */
        switch (argv[i][1])
        {
        case 'f':
            fileList = argv[++i];
            break;

        default:
            printf("Unrecognised argument %s\n", argv[i]);
            Usage();
            exit(EXIT_FAILURE);
        }
    }

    ASRFactory factory;
    Source* source;

    /* Use the ASR factory for the source and front-end */
    Plugin<float>* s = factory.CreateSource(source);
    Plugin<float>* f = factory.CreateFrontend(s);

    /* An HTK file sink */
    HTKSink sink(f);
    fflush(0);

    FilePath path;
    if (!fileList)
    {
        /* If there's no file list we need 2 files */
        if (fileCount < 2)
        {
            printf("Not enough files defined\n");
            exit(EXIT_FAILURE);
        }
        try
        {
            source->Open(file[0]);
            path.SetName(file[1]);
            path.MakePath();
            sink.Open(file[1]);
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
    }
    else
    {
        /* Extract a whole file list */
        assert(fileList);
        if (Tracter::sVerbose > 0)
            printf("filelist %s\n", fileList);
        FILE* list = fopen(fileList, "r");
        if (!list)
        {
            printf("Failed to open %s\n", fileList);
            exit(EXIT_FAILURE);
        }

        char file1[1024];
        char file2[1024];
        while (fscanf(list, "%s %s", file1, file2) == 2)
        {
            if (Tracter::sVerbose > 1)
                printf("raw: %s\nhtk: %s\n", file1, file2);
            sink.Reset();
            source->Open(file1);
            path.SetName(file2);
            path.MakePath();
            sink.Open(file2);
        }
        fclose(list);
    }

    return 0;
}
