/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Extract.h"
#include "FilePath.h"

Tracter::Extract::Extract(int iArgc, char** iArgv, ASRFactory* iFactory)
{
    mObjectName = "Extract";
    mFileList = 0;
    mFile[0] = 0;
    mFile[1] = 0;
    mLoop = false;

    /* Use the factory for the source and front-end */
    Component<float>* s = iFactory->CreateSource(mSource);
    Component<float>* f = iFactory->CreateFrontend(s);

    /* An HTK file sink */
    mSink = new HTKSink(f);

    /* Read command line for the files */
    int fileCount = 0;
    for (int i=1; i<iArgc; i++)
    {
        /* Unqualified arguments are the input and output files */
        if (iArgv[i][0] != '-')
        {
            if (fileCount < 2)
                mFile[fileCount++] = iArgv[i];
            else
                throw Exception("Too many unqualified arguments");
            continue;
        }

        /* Arguments beginning with a '-' */
        switch (iArgv[i][1])
        {
        case 'f':
            mFileList = iArgv[++i];
            break;

        case 'l':
            mLoop = true;
            break;

        case 'd':
            mSink->Dot();
            break;

        default:
            Usage(iArgv[0]);
            throw Exception("Unrecognised argument %s", iArgv[i]);
        }
    }
}

void Tracter::Extract::All()
{
    if (mFileList)
        List(mFileList);
    else
    {
        /* If there's no file list we need 2 files */
        if (!mFile[0] || !mFile[1])
            throw Exception("Not enough files defined");
        File(mFile[0], mFile[1], mLoop);
    }
}

Tracter::Extract::~Extract() throw ()
{
    delete mSink;
}

void Tracter::Extract::Usage(const char* iName)
{
    printf(
        "Usage: %s [options] [infile outfile | -f file-list]\n"
        "Options:\n"
        "-f list  Read input and output files from list\n"
        "-l       Loop indefinitely if not in list mode\n"
        "-d       Generate dot format graph\n"
        "Anything else prints this information\n"
        "Set environment variable Tracter_shConfig to 1 for more options\n",
        iName
    );
}

/**
 * Extract from one file to another.  If the loop parameter is true,
 * the sink is continually reset and cycled.
 */
void Tracter::Extract::File(
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
void Tracter::Extract::List(const char* iFileList)
{
    assert(iFileList);
    Verbose(1, "filelist %s\n", iFileList);
    FILE* list = fopen(iFileList, "r");
    if (!list)
        throw Exception("Failed to open %s", iFileList);


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
