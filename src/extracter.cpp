/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdlib.h>
#include <stdio.h>

#include "Signal.h"
#include "HTKSink.h"
#include "FilePath.h"
#include "ASRFactory.h"

namespace Tracter
{
    /**
     * Feature extractor
     *
     * Uses ASRFactory to construct a feature extractor with a source
     * and sink.  The sink is to HTK format files via HTKSink.
     */
    class Extracter : public Object
    {
    public:
        Extracter();
        virtual ~Extracter() throw ();
        void Extract(const char* iFile1, const char* iFile2, bool iLoop=false);
        void ExtractList(const char* iFileList);

    private:
        ISource* mSource;
        HTKSink* mSink;
    };
}

Tracter::Extracter::Extracter()
{
    mObjectName = "Extracter";

    /* Use the ASR factory for the source and front-end */
    ASRFactory factory;
    Plugin<float>* s = factory.CreateSource(mSource);
    Plugin<float>* f = factory.CreateFrontend(s);

    /* An HTK file sink */
    mSink = new HTKSink(f);
}

Tracter::Extracter::~Extracter() throw ()
{
    delete mSink;
}

/**
 * Extract from one file to another.  If the loop parameter is true,
 * the sink is continually reset and cycled.
 */
void Tracter::Extracter::Extract(
    const char* iFile1, const char* iFile2, bool iLoop
)
{
    mSource->Open(iFile1);
    FilePath path;
    path.SetName(iFile2);
    path.MakePath();
    do
    {
        mSink->Open(iFile2);
        mSink->Reset();
    }
    while (iLoop);
}

/**
 * Extract a file list.  The file list format is pairs of input and
 * output files.
 */
void Tracter::Extracter::ExtractList(const char* iFileList)
{
    assert(iFileList);
    Verbose(1, "filelist %s\n", iFileList);
    FILE* list = fopen(iFileList, "r");
    if (!list)
        throw Exception("Failed to open %s\n", iFileList);


    char file1[1024];
    char file2[1024];
    FilePath path;
    while (fscanf(list, "%s %s", file1, file2) == 2)
    {
        Verbose(1, "raw: %s\n", file1);
        Verbose(1, "htk: %s\n", file2);
        mSink->Reset();
        mSource->Open(file1);
        path.SetName(file2);
        path.MakePath();
        mSink->Open(file2);
    }
    fclose(list);
}

using namespace Tracter;

void Usage()
{
    puts(
        "Usage: extracter [options] [infile outfile | -f file-list]\n"
        "Options:\n"
        "-f list  Read input and output files from list\n"
        "-l       Loop indefinitely if not in list mode\n"
        "Anything else prints this information\n"
        "Set environment variable Tracter_shConfig to 1 for more options\n"
    );
}

/**
 * Extracter executable.
 */
int main(int argc, char** argv)
{
    /* Trap floating point exceptions */
    TrapFPE();

    /* Read command line */
    const char* fileList = 0;
    const char* file[2] = {0, 0};
    int fileCount = 0;
    bool loop = false;
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
                return 1;
            }
            continue;
        }

        /* Arguments beginning with a '-' */
        switch (argv[i][1])
        {
        case 'f':
            fileList = argv[++i];
            break;

        case 'l':
            loop = true;
            break;

        default:
            printf("Unrecognised argument %s\n", argv[i]);
            Usage();
            return 1;
        }
    }

    /*
     * It's debatable whether it's even worth catching these
     * exceptions.  If not caught, the same error messages appear via
     * the terminate call.  An example of how to do it perhaps?
     */
    try
    {
        Extracter extracter;
        if (fileList)
        {
            assert(fileList);
            extracter.ExtractList(fileList);
        }
        else
        {
            /* If there's no file list we need 2 files */
            if (fileCount < 2)
            {
                printf("Not enough files defined\n");
                return 1;
            }
            extracter.Extract(file[0], file[1], loop);
        }
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
