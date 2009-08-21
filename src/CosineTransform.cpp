/*
 * Copyright 2009 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "CosineTransform.h"

Tracter::CosineTransform::CosineTransform(
    Component<float>* iInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(mInput);

    mFrame.size = mInput->Frame().size;

    mIData = 0;
    mOData = 0;
    mDCT.Init(mFrame.size, &mIData, &mOData);

    mWindow.resize(mFrame.size);
    for (int i=0; i<mFrame.size; i++)
        mWindow[i] = 0.54f - 0.46f * cosf(M_PI * 2.0f * i / (mFrame.size - 1));
}

bool Tracter::CosineTransform::UnaryFetch(IndexType iIndex, float* oData)
{
    // Read the input frame
    const float* input = mInput->UnaryRead(iIndex);
    if (!input)
        return false;

    // Copy the frame into the pre-allocated array though a window
    for (int i=0; i<mFrame.size; i++)
        mIData[i] = input[i] * mWindow[i];

    // DCT
    mDCT.Transform();

    // Copy to output
    for (int i=0; i<mFrame.size; i++)
        oData[i] = mOData[i];

    return true;
}
