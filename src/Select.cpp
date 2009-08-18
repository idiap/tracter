/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Select.h"

Tracter::Select::Select(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;

    mLoIndex = GetEnv("Lo", 0);
    mHiIndex = GetEnv("Hi", iInput->GetArraySize()-1);

    mArraySize = mHiIndex - mLoIndex + 1;
    assert(mArraySize > 0);
    assert(mHiIndex < iInput->GetArraySize());
    assert(mLoIndex >= 0);

    MinSize(iInput, 1);

    Verbose(1, "passing indexes %d-%d of %d",
            mLoIndex, mHiIndex, iInput->GetArraySize());
}

bool Tracter::Select::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    float* output = GetPointer(iOffset);
    const float* input = mInput->UnaryRead(iIndex);
    if (!input)
        return false;

    // Copy input to output
    for (int i=mLoIndex; i<=mHiIndex; i++){
        output[i-mLoIndex] = input[i];
    }

    return true;
}

