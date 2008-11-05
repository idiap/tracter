/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "Periodogram.h"

/**
 * Periodogram variables:
 * - FrameSize (256)
 * - FramePeriod (80)
 */
Tracter::Periodogram::Periodogram(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mFrameSize = GetEnv("FrameSize", 256);
    mFramePeriod = GetEnv("FramePeriod", 80);
    mArraySize = mFrameSize/2+1;
    mSamplePeriod *= mFramePeriod;
    assert(mFrameSize > 0);
    assert(mFramePeriod > 0);

    // Framers look ahead, not back
    MinSize(mInput, mFrameSize, mFrameSize-1);
    mRealData = 0;
    mComplexData = 0;
    mFourier.Init(mFrameSize, &mRealData, &mComplexData);

    // Hardwire a Hamming window.  Could be generalised much better.
    // This one is asymmetric.  Should it be symmetric?
    const float PI = 3.14159265358979323846;
    mWindow.resize(mFrameSize);
    for (int i=0; i<mFrameSize; i++)
        mWindow[i] = 0.54f - 0.46f * cosf(PI * 2.0f * i / (mFrameSize - 1));
}

bool Tracter::Periodogram::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    int readIndex = iIndex * mFramePeriod;
    int got = mInput->Read(inputArea, readIndex, mFrameSize);
    if (got < mFrameSize)
        return false;

    // Copy the frame into the contiguous array
    float* p = mInput->GetPointer();
    for (int i=0; i<inputArea.len[0]; i++)
        mRealData[i] = p[inputArea.offset+i] * mWindow[i];
    for (int i=0; i<inputArea.len[1]; i++)
        mRealData[i+inputArea.len[0]] = p[i] * mWindow[i+inputArea.len[0]];

    // Do the DFT
    mFourier.Transform();

    // Compute periodogram
    float* cache = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
        cache[i] =
            mComplexData[i].real() * mComplexData[i].real() +
            mComplexData[i].imag() * mComplexData[i].imag();

    return true;
}
