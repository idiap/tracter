/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

//#include <cstdio>
//#include <cmath>
//#include <cfloat>

#include "Select.h"

Tracter::Select::Select(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;

    mLoIndex = GetEnv("Lo", 0);
    mHiIndex = GetEnv("Hi", -1);

    if (mHiIndex < 0)
    {
    	mHiIndex = iInput->GetArraySize() - 1;
    }

    mArraySize = mHiIndex - mLoIndex + 1;
    assert(mArraySize >= 0);
    assert(mHiIndex <= iInput->GetArraySize() - 1);
    assert(mLoIndex >= 0);

    // Keep all the input
    PluginObject::MinSize(iInput, 1);
}

bool Tracter::Select::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    CacheArea inputArea;
    if (mInput->Read(inputArea, iIndex) != 1){
      return false;
    }

    // Copy input to output with limits check
    float* input  = mInput->GetPointer(inputArea.offset);
    float* output = GetPointer(iOffset);
    for (int i=mLoIndex; i<=mHiIndex; i++){
      output[i-mLoIndex] = input[i];
    }

    return true;
}

