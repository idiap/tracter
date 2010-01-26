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

    if (GetEnv("Window", 0))
        mWindow = new Window(mObjectName, mFrame.size);
    else
        mWindow = 0;
}

bool Tracter::CosineTransform::UnaryFetch(IndexType iIndex, float* oData)
{
    // Read the input frame
    const float* ip = mInput->UnaryRead(iIndex);
    if (!ip)
        return false;

    if (mWindow)
        // Copy the frame via the window
        mWindow->Apply(ip, mIData);
    else
        // Raw copy
        for (int i=0; i<mFrame.size; i++)
            mIData[i] = ip[i];

    // DCT
    mDCT.Transform();

    // Copy to output
#if 1
    for (int i=0; i<mFrame.size; i++)
        oData[i] = mOData[i];
#else
    // A hack to simulate HTK feature order
    for (int i=0; i<12; i++)
        oData[i] = mOData[i+1];
    oData[12] = mOData[0];
#endif
    return true;
}
