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

    if (GetEnv("Window", 1))
        mWindow = new Window(mObjectName, frameSize);
    else
        mWindow = 0;
}

Tracter::Periodogram::~Periodogram() throw ()
{
    delete mWindow;
    mWindow = 0;
}

bool Tracter::Periodogram::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    // Read the input frame
    const float* p = mInput->UnaryRead(iIndex);
    if (!p)
        return false;

    if (mWindow)
        // Copy the frame via the window
        mWindow->Apply(p, mRealData);
    else
        // Raw copy
        for (int i=0; i<mInput->GetArraySize(); i++)
            mRealData[i] = p[i];

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
