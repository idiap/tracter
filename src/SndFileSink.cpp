/*
 * Copyright 2010 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "SndFileSink.h"

Tracter::SndFileSink::SndFileSink(
    Component<float>* iInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(mInput);
    mFrame.size = mInput->Frame().size;
    Initialise();
    Reset();

    mSndFile = 0;
    mFrameRate = GetEnv("FrameRate", 8000.0f);

    mFormat = SF_FORMAT_WAV;
    if (GetEnv("MAT5", 0)) mFormat = SF_FORMAT_MAT5;
    if (GetEnv("NIST", 0)) mFormat = SF_FORMAT_NIST;
}

/**
 * Opens the given file and sucks data into it.
 */
void Tracter::SndFileSink::Open(const char* iFile)
{
    assert(iFile);
    assert(!mSndFile);

    /* Open the file */
    SF_INFO sfInfo;
    sfInfo.format = mFormat | SF_FORMAT_PCM_16 | SF_ENDIAN_FILE;
    sfInfo.samplerate = (int)mFrameRate;
    sfInfo.channels = mFrame.size;
    Verbose(1, "%s\n", iFile);
    mSndFile = sf_open(iFile, SFM_WRITE, &sfInfo);
    if (!mSndFile)
        throw Exception("SndFileSink::Open: failed on %s", iFile);

    /* Pull all data */
    int index = 0;
    CacheArea cache;
    while (mInput->Read(cache, index++))
    {
        float* f = mInput->GetPointer(cache.offset);
        int nPut = (int)sf_writef_float(mSndFile, f, mFrame.size);
        if (nPut != 1)
            throw Exception("SndFileSink::Open: failed to write index %d",
                            index);
    }

    /* Close the file */
    sf_close(mSndFile);
}
