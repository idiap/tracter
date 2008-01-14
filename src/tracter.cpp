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
#include "Concatenate.h"
#include "Delta.h"
//#include "PLP.h"
#include "Pixmap.h"
#include "ComplexSample.h"
#include "ComplexPeriodogram.h"
#include "FilePath.h"

/**
 * A basic ASR front-end with pre-emphasis, spectral subtraction and
 * cepstral mean subtraction.
 *
 * ...except that spectral subtraction doesn't work well yet
 */
Plugin<float>* BasicFrontend(Plugin<float>* iSource)
{
    /* Signal processing chain */
    ZeroFilter* zf = new ZeroFilter(iSource);
    Periodogram* p = new Periodogram(zf);
    //Noise* nn = new Noise(p);
    //SpectralSubtract *ss = new SpectralSubtract(p, nn);
    MelFilter* mf = new MelFilter(p);
    Cepstrum* c = new Cepstrum(mf);
    Mean* m = new Mean(c);
    Subtract* s = new Subtract(c, m);
    return s;
}

#if 0
Plugin<float>* PLPFrontend(Plugin<float>* iSource)
{
    /* Signal processing chain */
    ZeroFilter* zf = new ZeroFilter(iSource);
    Periodogram* p = new Periodogram(zf);
    MelFilter* mf = new MelFilter(p);
    PLP* l = new PLP(mf);
    //Pixmap* pm = new Pixmap(l);
    Mean* m = new Mean(l);
    Subtract* s = new Subtract(l, m);
    return s;
}
#endif

Plugin<float>* ComplexFrontend(Plugin<float>* iSource)
{
    /* Signal processing chain */
    ComplexSample* cs = new ComplexSample(iSource);
    ComplexPeriodogram* cp = new ComplexPeriodogram(cs);
    Pixmap* pm = new Pixmap(cp);
    return pm;
}

void Usage()
{
    puts(
        "Usage: tracter [options] [infile outfile | -f file-list]\n"
        "Options:\n"
        "-v      Increment verbosity level (e.g., -v -v -v sets it to 3)\n"
        "-c      Dump the configuration parameters to stdout\n"
        "-d n    Add deltas up to order n\n"
        "-f list Read input and output files from list\n"
        "Anything else prints this information\n"
    );
}

/**
 * Tracter executable.
 */
int main(int argc, char** argv)
{
    bool verbose = false;
    int deltaOrder = 0;

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
        case 'v':
            verbose = true;
            Tracter::sVerbose++;
            break;

        case 'f':
            fileList = argv[++i];
            break;

        case 'd':
            deltaOrder = atoi(argv[++i]);
            break;

        case 'c':
            Tracter::sShowConfig = true;
            break;

        default:
            printf("Unrecognised argument %s\n", argv[i]);
            Usage();
            exit(EXIT_FAILURE);
        }
    }

    /* Raw file source and normaliser for 16 bit files */
    FileSource<short>* source = new FileSource<short>();
    Normalise* n = new Normalise(source);

    /* Choose a front-end architecture */
    Plugin<float>* f = BasicFrontend(n);
    //Plugin<float>* f = ComplexFrontend(n);
    //Plugin<float>* f = PLPFrontend(n);

    /* Add deltas up to deltaOrder */
    if (deltaOrder > 0)
    {
        Concatenate* c = new Concatenate();
        c->Add(f);
        for (int i=0; i<deltaOrder; i++)
        {
            Delta* d = new Delta(f);
            c->Add(d);
            f = d;
        }
        f = c;
    }

    /* An HTK file sink */
    HTKSink sink(f);
    FilePath path;

    if (!fileList)
    {
        /* If there's no file list we need 2 files */
        if (fileCount < 2)
        {
            printf("Not enough files defined\n");
            exit(EXIT_FAILURE);
        }
        source->Open(file[0]);
        path.SetName(file[1]);
        path.MakePath();
        sink.Open(file[1]);
    }
    else
    {
        /* Extract a whole file list */
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
            path.SetName(file2);
            path.MakePath();
            sink.Open(file2);
        }
        fclose(list);
    }

    return 0;
}
