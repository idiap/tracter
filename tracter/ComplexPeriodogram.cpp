/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "ComplexPeriodogram.h"

Tracter::ComplexPeriodogram::ComplexPeriodogram(
    Component<complex>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    mFrame.period = config("FramePeriod", 40);
    Connect(iInput);
    mFrame.size = config("FrameSize", 128);
    assert(mFrame.size > 0);
    assert(mFrame.period > 0);

    ComponentBase::MinSize(mInput, mFrame.size, mFrame.size-1);

    mInputData = 0;
    mOutputData = 0;
    mFourier.Init(mFrame.size, &mInputData, &mOutputData);

    // Hardwire a Hamming window.  Could be generalised much better.
    // This one is symmetric.  Should it be asymmetric?
    const float PI = 3.14159265358979323846;
    mWindow.resize(mFrame.size);
    for (int i=0; i<mFrame.size; i++)
        mWindow[i] = 0.54f - 0.46f * cosf(PI * 2.0f * i / (mFrame.size - 1));
}

bool Tracter::ComplexPeriodogram::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    int readIndex = (int)(mFrame.period * iIndex);
    int got = mInput->Read(inputArea, readIndex, mFrame.size);
    if (got < mFrame.size)
        return false;

    // Copy the frame into the contiguous array
    complex* p = mInput->GetPointer();
    complex* pp = reinterpret_cast<complex*>(mInputData);
    for (int i=0; i<inputArea.len[0]; i++)
        pp[i] = p[inputArea.offset+i] * mWindow[i];
    for (int i=0; i<inputArea.len[1]; i++)
        pp[i+inputArea.len[0]] = p[i] * mWindow[i+inputArea.len[0]];

    // Do the DFT
    mFourier.Transform();

    // Compute periodogram
    for (int i=0; i<mFrame.size; i++)
        oData[i] =
            mOutputData[i].real() * mOutputData[i].real() +
            mOutputData[i].imag() * mOutputData[i].imag();

    return true;
}
