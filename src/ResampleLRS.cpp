/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>
#include <vector>

#include <libresample.h>

#include "Resample.h"

/**
 * The class specific data for ResampleLRS
 */
struct Tracter::ResampleData
{
    void *handle;
    double ratio;
    std::vector<float> resample;
};

/**
 * libresample (LRS) based resampler.  Depends on libresample, which
 * can be downloaded from http://ccrma.stanford.edu/~jos/resample/.
 *
 * Initialise a sample rate converter.  Environment variables:
 *  - TargetFreq (16000)  Target sample frequency.
 */
Tracter::Resample::Resample(
    Component<float>* iInput, const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;

    double targetRate = GetEnv("TargetRate", 16000);
    mResampleData = new ResampleData;
    ResampleData& r = *mResampleData;
    r.ratio = targetRate / mInput->FrameRate();
    r.handle = resample_open(1, r.ratio, r.ratio);
    assert(r.handle);

    Connect(mInput, 1.0f / r.ratio);
}

/**
 * Deletes the sample rate converter
 */
Tracter::Resample::~Resample() throw()
{
    ResampleData& r = *mResampleData;
    resample_close(r.handle);
    delete mResampleData;
    mResampleData = 0;
}

/**
 * Ensures that the input component has the right size given the rate conversion
 */
void Tracter::Resample::MinSize(int iSize, int iReadBehind, int iReadAhead)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    ComponentBase::MinSize(iSize, iReadBehind, iReadAhead);

    // Set the input buffer big enough to service largest output requests
    assert(mInput);
    ResampleData& r = *mResampleData;
    int minSize = (int)((double)iSize / r.ratio + 0.5);
    ComponentBase::MinSize(mInput, minSize, 0, 0);

    /* It's too complicated without an intermediate array */
    r.resample.resize(iSize);
}

void Tracter::Resample::Reset(bool iPropagate)
{
    // Do a reset as a close then open
    ResampleData& r = *mResampleData;
    resample_close(r.handle);
    r.handle = resample_open(0, r.ratio, r.ratio);
    CachedComponent<float>::Reset(iPropagate);
}

int Tracter::Resample::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iOutputArea.Length() > 0);
    assert(iIndex >= 0);
    CacheArea inputArea;

    ResampleData& r = *mResampleData;
    IndexType index = (IndexType)((double)iIndex / r.ratio);
    int nGet = (int)((double)iOutputArea.Length() / r.ratio);
    int nGot = mInput->Read(inputArea, index, nGet);
    int nOut = (int)((double)nGot * r.ratio + 0.5);
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
            r.handle,
            r.ratio,
            mInput->GetPointer(inputArea.offset) + inCount,
            inputArea.len[0] - inCount,
            ((nGot < nGet) && (inputArea.len[1] == 0)),
            &inCount,
            &r.resample.at(outCount),
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
                r.handle,
                r.ratio,
                mInput->GetPointer() + inCount,
                inputArea.len[1] - inCount,
                (nGot < nGet),
                &inCount,
                &r.resample.at(outCount),
                nOut - outCount
            );
        Verbose(2, " Block 2: inCount %d  outCount %d\n", inCount, outCount);
    }
    else
    {
        while (outCount < nOut)
            outCount += resample_process(
                r.handle,
                r.ratio,
                mInput->GetPointer(inputArea.offset) + inCount,
                inputArea.len[0] - inCount,
                (nGot < nGet),
                &inCount,
                &r.resample.at(outCount),
                nOut - outCount
            );
        Verbose(2, " Block 3: inCount %d  outCount %d\n", inCount, outCount);
    }

    /* Copy the resampled data to the output */
    float* output = GetPointer(iOutputArea.offset);
    for (int i=0; i<iOutputArea.len[0]; i++)
        output[i] = r.resample[i];
    output = GetPointer();
    for (int i=0; i<iOutputArea.len[1]; i++)
        output[i] = r.resample[iOutputArea.len[0] + i];

    return nOut;
}
