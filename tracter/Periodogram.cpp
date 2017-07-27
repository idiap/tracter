/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "Periodogram.h"

Tracter::Periodogram::Periodogram(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(mInput);

    int frameSize = mInput->frame().size;
    mFrame.size = frameSize/2+1;

    mRealData = 0;
    mComplexData = 0;
    mFourier.init(frameSize, &mRealData, &mComplexData);

    if (config("Window", 1))
        mWindow = new Window(objectName(), frameSize);
    else
        mWindow = 0;
}

Tracter::Periodogram::~Periodogram() throw ()
{
    delete mWindow;
    mWindow = 0;
}

bool Tracter::Periodogram::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame
    const float* p = mInput->unaryRead(iIndex);
    if (!p)
        return false;

    if (mWindow)
        // Copy the frame via the window
        mWindow->apply(p, mRealData);
    else
        // Raw copy
        for (int i=0; i<mInput->frame().size; i++)
            mRealData[i] = p[i];

    // Do the DFT
    mFourier.transform();

    // Compute periodogram
    for (int i=0; i<mFrame.size; i++)
        oData[i] =
            mComplexData[i].real() * mComplexData[i].real() +
            mComplexData[i].imag() * mComplexData[i].imag();

    return true;
}
