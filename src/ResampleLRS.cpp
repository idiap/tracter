/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>

#include <libresample.h>

#include "Resample.h"

/**
 * libresample (LRS) based resampler.  Depends on libresample, which
 * can be downloaded from http://ccrma.stanford.edu/~jos/resample/.
 *
 * Initialise a sample rate converter.  Environment variables:
 *  - TargetFreq (16000)  Target sample frequency.
 */
Tracter::Resample::Resample(
    Plugin<float>* iInput, const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mSampleFreq = GetEnv("TargetFreq", 16000);

    mRatio = (double)mSampleFreq / mInput->GetSampleFreq();
    mHandle = resample_open(1, mRatio, mRatio);
    assert(mHandle);
}

/**
 * Deletes the sample rate converter
 */
Tracter::Resample::~Resample() throw()
{
    resample_close(mHandle);
    mHandle = 0;
}

/**
 * Ensures that the input plugin has the right size given the rate conversion
 */
void Tracter::Resample::MinSize(int iSize, int iReadBack, int iReadAhead)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    PluginObject::MinSize(iSize, iReadBack, iReadAhead);

    // Set the input buffer big enough to service largest output requests
    assert(mInput);
    int minSize = (int)((double)iSize / mRatio + 0.5);
    PluginObject::MinSize(mInput, minSize, 0, 0);

    /* It's too complicated without an intermediate array */
    mResample.resize(iSize);
}

void Tracter::Resample::Reset(bool iPropagate)
{
    // Do a reset as a close then open
    resample_close(mHandle);
    mHandle = resample_open(0, mRatio, mRatio);
    UnaryPlugin<float, float>::Reset(iPropagate);
}

int Tracter::Resample::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iOutputArea.Length() > 0);
    assert(iIndex >= 0);
    CacheArea inputArea;

    IndexType index = (IndexType)((double)iIndex / mRatio);
    int nGet = (int)((double)iOutputArea.Length() / mRatio);
    int nGot = mInput->Read(inputArea, index, nGet);
    int nOut = (int)((double)nGot * mRatio + 0.5);
    Verbose(2, "i=%ld Get=%d Got=%d Out=%d len0=%d len1=%d\n",
            index, nGet, nGot, nOut, inputArea.len[0], inputArea.len[1]);

    /*
     * The function resample_process doesn't necessarily suck in all
     * its input, or produce all the output you might expect, hence
     * these daft loops.  At the end, I guess because of filter
     * lookahead, it can produce output without consuming any input at
     * all; that's why the (outCount < nOut) is there on the first
     * loop.
     */

    /* Run it over the first circular block */
    int inCount = 0;
    int outCount = 0;
    int i = 0;
    while ((inCount < inputArea.len[0]) && (outCount < nOut) && (i++ < 10))
    {
        outCount += resample_process(
            mHandle,
            mRatio,
            mInput->GetPointer(inputArea.offset) + inCount,
            inputArea.len[0] - inCount,
            ((nGot < nGet) && (inputArea.len[1] == 0)),
            &inCount,
            &mResample.at(outCount),
            nOut - outCount
        );
        Verbose(2, " Block 1: inCount %d  outCount %d\n", inCount, outCount);
    }

    /* Run again if there's a second circular block */
    if (inputArea.len[1])
    {
        inCount = 0;
        while (outCount < nOut)
            outCount += resample_process(
                mHandle,
                mRatio,
                mInput->GetPointer() + inCount,
                inputArea.len[1] - inCount,
                (nGot < nGet),
                &inCount,
                &mResample.at(outCount),
                nOut - outCount
            );
        Verbose(2, " Block 2: inCount %d  outCount %d\n", inCount, outCount);
    }
    else
    {
        while (outCount < nOut)
            outCount += resample_process(
                mHandle,
                mRatio,
                mInput->GetPointer(inputArea.offset) + inCount,
                inputArea.len[0] - inCount,
                (nGot < nGet),
                &inCount,
                &mResample.at(outCount),
                nOut - outCount
            );
        Verbose(2, " Block 3: inCount %d  outCount %d\n", inCount, outCount);
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
