/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Extract.h"
#include "FilePath.h"

#include <lube/config.h>

using namespace lube;

Tracter::Extract::Extract(int iArgc, char** iArgv, ASRFactory* iFactory)
{
    objectName("Extract");
    mLoop = false;

    // Sort out the command line
    Option o("Extract: Extract features to file");
    o('C', "Read configuration file", "");
    o('v', "Set verbosity level", 0);
    o('f', "Read input and output files from list", "");
    o('l', "Loop indefinitely if not in list mode");
    o('d', "Generate dot format graph");
    o.parse(iArgc, iArgv);

    // Handle the qualified arguments
    if (o['v'].defined())
        verbose(o['v']);
    if (o['C'] != "")
        configFile(o['C']);
    if (o['f'] != "")
        mFileList = o['f'];
    if (o['l'].defined())
        mLoop = true;
    if (o['d'].defined())
        mSink->dot();

    // Unqualified arguments are then filenames
    var argv = o.args();
    if (argv.size() == 2)
    {
        mFile[0] = argv[0];
        mFile[1] = argv[1];
    }
    if (argv.size() > 2)
        throw Exception("Too many unqualified arguments");

    /* Use the factory for the source and front-end; add an HTK sink */
    Component<float>* s = iFactory->createSource(mSource);
    Component<float>* f = iFactory->createFrontend(s);
    mSink = new HTKSink(f);
}

void Tracter::Extract::All()
{
    if (mFileList)
        List(mFileList.str());
    else
    {
        /* If there's no file list we need 2 files */
        if (!mFile[0] || !mFile[1])
            throw Exception("Not enough files defined");
        File(mFile[0].str(), mFile[1].str(), mLoop);
    }
}

Tracter::Extract::~Extract() throw ()
{
    delete mSink;
}

/**
 * Extract from one file to another.  If the loop parameter is true,
 * the sink is continually reset and cycled.
 */
void Tracter::Extract::File(
    const char* iFile1, const char* iFile2, bool iLoop
)
{
    assert(iFile1);
    assert(iFile2);
    verbose(1, "file1 %s, file2 %s\n", iFile1, iFile2);
    mSource->open(iFile1);
    FilePath path;
    path.SetName(iFile2);
    path.MakePath();
    do
    {
        mSink->open(iFile2);
        mSink->reset();
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
    verbose(1, "filelist %s\n", iFileList);
    FILE* list = fopen(iFileList, "r");
    if (!list)
        throw Exception("Failed to open %s", iFileList);


    char file1[1024];
    char file2[1024];
    FilePath path;
    while (fscanf(list, "%s %s", file1, file2) == 2)
    {
        verbose(1, "raw: %s\n", file1);
        verbose(1, "htk: %s\n", file2);
        mSink->reset();
        mSource->open(file1);
        path.SetName(file2);
        path.MakePath();
        mSink->open(file2);
    }
    fclose(list);
}
