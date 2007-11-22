/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdlib.h>
#include <stdio.h>

#include "FileSource.h"
#include "Normalise.h"
#include "ZeroFilter.h"
#include "Periodogram.h"
#include "MelFilter.h"
#include "Cepstrum.h"
#include "HTKSink.h"
#include "Mean.h"
#include "Subtract.h"
#include "SpectralSubtract.h"
#include "Noise.h"

int main(int argc, char** argv)
{
    bool verbose = false;
    bool dumpMel = false;

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

        // Arguments beginning with a '-'
        switch (argv[i][1])
        {
        case 'v':
            verbose = true;
            break;

        case 'f':
            fileList = argv[++i];
            break;

        case 'm':
            dumpMel = true;
            break;

        case 'c':
            Tracter::sShowConfig = true;
            break;

        default:
            printf("Unrecognised argument %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    /* Raw file source and normaliser for 16 bit big endian files */
    FileSource<short>* source = new FileSource<short>();
    Normalise* n = new Normalise(source);

    /* Signal processing chain */
    ZeroFilter* zf = new ZeroFilter(n);
    Periodogram* p = new Periodogram(zf);
    Noise* nn = new Noise(p);
    SpectralSubtract *ss = new SpectralSubtract(p, nn);
    MelFilter* mf = new MelFilter(ss);
    Cepstrum* c = new Cepstrum(mf);
    Mean* m = new Mean(c);
    Subtract* s = new Subtract(c, m);

    /* An HTK file sink */
    HTKSink sink(s);

    if (dumpMel)
    {
        mf->DumpBins();
        exit(EXIT_SUCCESS);
    }

    /* For now we need 2 files */
    if (!fileList && (fileCount < 2))
    {
        printf("Not enough files defined\n");
        exit(EXIT_FAILURE);
    }

    if (!fileList)
    {
        // Just run it over one input and output
        source->Open(file[0]);
        sink.Open(file[1]);
    }
    else
    {
        // Extract a whole file list
        assert(fileList);
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
            if (verbose)
                printf("%s\n %s\n", file1, file2);
            sink.Reset();
            source->Open(file1);
            sink.Open(file2);
        }
        fclose(list);
    }

    return 0;
}
