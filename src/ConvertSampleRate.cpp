/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ConvertSampleRate.h"

/**
 * Initialise a sample rate converter.  Environment variables:
 *  - TargetFreq (16000)  Target sample frequency.
 */
ConvertSampleRate::ConvertSampleRate(
    Plugin<float>* iInput, const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mSampleFreq = GetEnv("TargetFreq", 16000);
    mData.src_ratio = (double)mSampleFreq / mInput->GetSampleFreq();
    assert(src_is_valid_ratio(mData.src_ratio));

    int error;
    mState = src_new(0, 1, &error);
    if (!mState)
    {
        printf("%s: SRC error %s\n", mObjectName, src_strerror(error));
        exit(EXIT_FAILURE);
    }
}

/**
 * Deletes the sample rate converter
 */
ConvertSampleRate::~ConvertSampleRate()
{
    src_delete(mState);
    mState = 0;
}

/**
 * Ensures that the input plugin has the right size given the rate conversion
 */
void ConvertSampleRate::MinSize(int iSize, int iReadAhead)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    PluginObject::MinSize(iSize, iReadAhead);

    // Set the input buffer big enough to service largest output requests
    assert(mInput);
    PluginObject::MinSize(
        mInput, (int)((double)iSize / mData.src_ratio + 0.5), 0
    );
}

void ConvertSampleRate::Reset(bool iPropagate)
{
    src_reset(mState);
    UnaryPlugin<float, float>::Reset(iPropagate);
}

int ConvertSampleRate::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iOutputArea.Length() > 0);
    assert(iIndex >= 0);
    CacheArea inputArea;

    int nGet = (int)((double)iOutputArea.Length() / mData.src_ratio);
    int nGot = mInput->Read(inputArea, iIndex, nGet);
    int nOut = (int)((double)nGot * mData.src_ratio);
    assert(nOut / mData.src_ratio == nGot);

    /* It's too complcated without an intermediate array */
    mResample.resize(nOut);

    /* Run it over the first circular block */
    mData.data_in = mInput->GetPointer(inputArea.offset);
    mData.data_out = &mResample[0];
    mData.input_frames = inputArea.len[0];
    mData.output_frames = mResample.size();
    mData.end_of_input = ((nGot < nGet) && (inputArea.len[1] == 0));
    process();

    /* Run again if there's a second circular block */
    if (inputArea.len[1] > 0)
    {
        mData.data_in = mInput->GetPointer();
        mData.data_out = &mResample[mData.output_frames_gen];
        mData.input_frames = inputArea.len[1];
        mData.output_frames = mResample.size()-mData.output_frames_gen;
        mData.end_of_input = (nGot < nGet);
        process();
    }

    /* Copy the resampled data to the output */
    float* output = GetPointer(iOutputArea.offset);
    for (int i=0; i<iOutputArea.len[0]; i++)
        output[i] = mResample[i];
    output = GetPointer();
    for (int i=0; i<iOutputArea.len[1]; i++)
        output[i] = mResample[iOutputArea.len[0] + i];

    return nOut;
}

/**
 * Runs src_process() and checks the error code.
 */
void ConvertSampleRate::process()
{
    int error = src_process(mState, &mData);
    if (error)
    {
        printf("%s: src_process error %s\n", mObjectName, src_strerror(error));
        exit(EXIT_FAILURE);
    }
}
