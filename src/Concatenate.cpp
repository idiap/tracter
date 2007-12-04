/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Concatenate.h"

PluginObject* Concatenate::GetInput(int iInput)
{
    assert(iInput < mNInputs);
    return mInput[iInput];
}

Concatenate::Concatenate(const char* iObjectName)
    : CachedPlugin<float>()
{
    mObjectName = iObjectName;
    mArraySize = 0;
}

void Concatenate::Add(Plugin<float>* iInput)
{
    mInput.push_back(iInput);
    mLength.push_back(iInput->GetArraySize());
    mArraySize += iInput->GetArraySize();
    Connect(iInput);
    MinSize(iInput, 1);
}

bool Concatenate::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);
    CacheArea inputArea;

    // Start with the high order inputs as they are likely to be Deltas
    // and it will help to prevent backwards cache reads.
    float* cache = GetPointer(iOffset);
    int arrayPos = mArraySize;
    for (int i=mNInputs-1; i>=0; i--)
    {
        if(mInput[i]->Read(inputArea, iIndex) == 0)
            return false;
        arrayPos -= mLength[i];
        assert(arrayPos >= 0);
        float* p = mInput[i]->GetPointer(inputArea.offset);
        for (int j=0; j<mLength[i]; j++)
            cache[arrayPos+j] = p[j];
    }

    return true;
}
