/*
 * Copyright 2010 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "FourierTransform.h"

Tracter::FourierTransformR2C::FourierTransformR2C(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    Connect(mInput);

    int frameSize = mInput->Frame().size;
    mFrame.size = frameSize/2+1;

    mRealData = 0;
    mComplexData = 0;
    mFourier.Init(frameSize, &mRealData, &mComplexData);

    if (GetEnv("Window", 1))
        mWindow = new Window(objectName(), frameSize);
    else
        mWindow = 0;

    Verbose(1, "R x %d to C x %d\n", frameSize, mFrame.size);
}

Tracter::FourierTransformR2C::~FourierTransformR2C() throw ()
{
    delete mWindow;
    mWindow = 0;
}

bool Tracter::FourierTransformR2C::UnaryFetch(IndexType iIndex, complex* oData)
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
        for (int i=0; i<mInput->Frame().size; i++)
            mRealData[i] = p[i];

    // Do the DFT
    mFourier.Transform();

    // Copy to output
    for (int i=0; i<mFrame.size; i++)
        oData[i] = mComplexData[i];

    return true;
}


Tracter::FourierTransformC2R::FourierTransformC2R(
    Component<complex>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    Connect(mInput);

    int frameSize = mInput->Frame().size;
    mFrame.size = (frameSize-1)*2;

    mRealData = 0;
    mComplexData = 0;
    mFourier.Init(mFrame.size, &mComplexData, &mRealData);

    if (GetEnv("Window", 1))
    {
        mWindow = new Window(objectName(), mFrame.size);
        mWindow->Scale(1.0f/mFrame.size);
    }
    else
        mWindow = 0;

    Verbose(1, "R x %d to C x %d\n", frameSize, mFrame.size);
}

Tracter::FourierTransformC2R::~FourierTransformC2R() throw ()
{
    delete mWindow;
    mWindow = 0;
}

bool Tracter::FourierTransformC2R::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame
    const complex* p = mInput->UnaryRead(iIndex);
    if (!p)
        return false;

    // Copy to input frame of DFT
    int frameSize = mInput->Frame().size;
    for (int i=0; i<frameSize; i++)
        mComplexData[i] = p[i];

    // Do the DFT
    mFourier.Transform();

    if (mWindow)
        // Copy the frame via the window
        mWindow->Apply(mRealData, oData);
    else
        // Raw copy
        for (int i=0; i<mFrame.size; i++)
            oData[i] = mRealData[i] / mFrame.size;

    return true;
}
