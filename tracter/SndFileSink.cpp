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
    objectName(iObjectName);
    mInput = iInput;

    mSndFile = 0;
    mBlockSize = GetEnv("BlockSize", 256);
    Connect(mInput, mBlockSize);

    mFrameRate = GetEnv("FrameRate", FrameRate());
    mFrame.size = mInput->Frame().size;
    Initialise();
    Reset();

    // There are lots; see: http://www.mega-nerd.com/libsndfile/api.html
    const StringEnum cFormat[] = {
        {"WAV",  SF_FORMAT_WAV},
        {"MAT5", SF_FORMAT_MAT5},
        {"NIST", SF_FORMAT_NIST},
        {"FLAC", SF_FORMAT_FLAC},
        {0, -1}
    };
    mFormat = GetEnv(cFormat, SF_FORMAT_WAV);
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
