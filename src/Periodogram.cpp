/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "Periodogram.h"

Tracter::Periodogram::Periodogram(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    int frameSize = mInput->GetArraySize();
    mArraySize = frameSize/2+1;
    MinSize(mInput, 1);

    mRealData = 0;
    mComplexData = 0;
    mFourier.Init(frameSize, &mRealData, &mComplexData);

    // Hardwire a Hamming window.  Could be generalised much better.
    // This one is asymmetric.  Should it be symmetric?
    const float PI = 3.14159265358979323846;
    mWindow.resize(frameSize);
    for (int i=0; i<frameSize; i++)
        mWindow[i] = 0.54f - 0.46f * cosf(PI * 2.0f * i / (frameSize - 1));
}

bool Tracter::Periodogram::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    // Read the input frame
    const float* p = mInput->UnaryRead(iIndex);
    if (!p)
        return false;

    // Copy the frame via the window
    for (size_t i=0; i<mWindow.size(); i++)
        mRealData[i] = p[i] * mWindow[i];

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
