/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Select.h"

Tracter::Select::Select(Component<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mInput = iInput;

    mLoIndex = GetEnv("Lo", 0);
    mHiIndex = GetEnv("Hi", iInput->Frame().size-1);

    mFrame.size = mHiIndex - mLoIndex + 1;
    assert(mFrame.size >= 0);
    assert(mHiIndex < iInput->Frame().size);
    assert(mLoIndex >= 0);
    Verbose(1, "passing indexes %d-%d of %d",
            mLoIndex, mHiIndex, iInput->Frame().size);

    Connect(iInput, 1);
}

bool Tracter::Select::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    const float* input = mInput->UnaryRead(iIndex);
    if (!input)
        return false;

    // Copy input to output
    for (int i=mLoIndex; i<=mHiIndex; i++){
        oData[i-mLoIndex] = input[i];
    }

    return true;
}

