/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "SndFileSource.h"

Tracter::SndFileSource::SndFileSource(const char* iObjectName)
{
    mObjectName = iObjectName;
    mArraySize = GetEnv("ArraySize", 1);
    mSampleFreq = GetEnv("SampleFreq", 8000.0f);
    mSamplePeriod = 1;
    mSndFile = 0;
    mNFrames = 0;
}

Tracter::SndFileSource::~SndFileSource() throw ()
{
    if (mSndFile)
        sf_close(mSndFile);
    mSndFile = 0;
    mNFrames = 0;
}

/**
 * Open a SndFile source
 */
void Tracter::SndFileSource::Open(const char* iFileName)
{
    assert(iFileName);

    if (mSndFile)
        sf_close(mSndFile);
    mSndFile = 0;
    mNFrames = 0;

    SF_INFO sfInfo;
    sfInfo.format = 0;
    mSndFile = sf_open(iFileName, SFM_READ, &sfInfo);
    if (!mSndFile)
        throw Exception("Failed to open file %s", iFileName);

    mSampleFreq = sfInfo.samplerate;
    mNFrames = sfInfo.frames;
    Verbose(1, "samplerate = %f\n", mSampleFreq);
    Verbose(1, "frames =     %d\n", mNFrames);
}

int Tracter::SndFileSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    // Seek to the right point, unless it's off the end
    if (iIndex > mNFrames)
        return 0;
    if (sf_seek(mSndFile, iIndex, SEEK_SET) < 0)
        throw Exception("Seek failed at index %ld", iIndex);

    // First circular block
    float* cache = GetPointer(iOutputArea.offset);
    int nGot = sf_readf_float(mSndFile, cache, iOutputArea.len[0]);
    if (nGot < iOutputArea.len[0])
        return nGot;

    // Second circular block
    if (iOutputArea.len[1])
    {
        cache = GetPointer();
        nGot = sf_readf_float(mSndFile, cache, iOutputArea.len[1]);
        if (nGot < iOutputArea.len[1])
            return nGot + iOutputArea.len[0];
    }

    // Done
    return iOutputArea.Length();
}
