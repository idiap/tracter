/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Select.h"

Tracter::Select::Select(Component<float>* iInput, const char* iObjectName)
{
    objectName(iObjectName);
    mInput = iInput;

    mLoIndex = config("Lo", 0);
    mHiIndex = config("Hi", iInput->frame().size-1);

    mFrame.size = mHiIndex - mLoIndex + 1;
    assert(mFrame.size >= 0);
    assert(mHiIndex < iInput->frame().size);
    assert(mLoIndex >= 0);
    verbose(1, "passing indexes %d-%d of %d\n",
            mLoIndex, mHiIndex, iInput->frame().size);

    connect(iInput, 1);
}

bool Tracter::Select::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    const float* input = mInput->unaryRead(iIndex);
    if (!input)
        return false;

    // Copy input to output
    for (int i=mLoIndex; i<=mHiIndex; i++){
        oData[i-mLoIndex] = input[i];
    }

    return true;
}

