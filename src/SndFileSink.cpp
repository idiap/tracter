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

    mSndFile = 0;
    mFrameRate = GetEnv("FrameRate", 8000.0f);
    mBlockSize = GetEnv("BlockSize", 256);
    Connect(mInput, mBlockSize);

    mFrame.size = mInput->Frame().size;
    Initialise();
    Reset();

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
    while (int nGot = mInput->Read(cache, index, mBlockSize))
    {
        float* ip = mInput->GetPointer();
        int nPut = 0;
        nPut += (int)sf_writef_float(mSndFile, ip+cache.offset, cache.len[0]);
        nPut += (int)sf_writef_float(mSndFile, ip, cache.len[1]);
        if (nPut != nGot)
            throw Exception("SndFileSink::Open: failed to write index %d",
                            index);
        index += nGot;
    }

    /* Close the file */
    sf_close(mSndFile);
}
