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
    mBlockSize = config("BlockSize", 256);
    connect(mInput, mBlockSize);

    mFrameRate = config("FrameRate", frameRate());
    mFrame.size = mInput->frame().size;
    initialise();
    reset();

    // There are lots; see: http://www.mega-nerd.com/libsndfile/api.html
    const StringEnum cFormat[] = {
        {"WAV",  SF_FORMAT_WAV},
        {"MAT5", SF_FORMAT_MAT5},
        {"NIST", SF_FORMAT_NIST},
        {"FLAC", SF_FORMAT_FLAC},
        {0, -1}
    };
    mFormat = config(cFormat, SF_FORMAT_WAV);
}

/**
 * Opens the given file and sucks data into it.
 */
void Tracter::SndFileSink::open(const char* iFile)
{
    assert(iFile);
    assert(!mSndFile);

    /* Open the file */
    SF_INFO sfInfo;
    sfInfo.format = mFormat | SF_FORMAT_PCM_16 | SF_ENDIAN_FILE;
    sfInfo.samplerate = (int)mFrameRate;
    sfInfo.channels = mFrame.size;
    verbose(1, "%s\n", iFile);
    mSndFile = sf_open(iFile, SFM_WRITE, &sfInfo);
    if (!mSndFile)
        throw Exception("SndFileSink::Open: failed on %s", iFile);

    /* Pull all data */
    int index = 0;
    CacheArea cache;
    while (int nGot = mInput->read(cache, index, mBlockSize))
    {
        float* ip = mInput->getPointer();
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
