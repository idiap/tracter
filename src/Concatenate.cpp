/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Concatenate.h"

Tracter::Concatenate::Concatenate(const char* iObjectName)
{
    mObjectName = iObjectName;
    mFrame.size = 0;
}

void Tracter::Concatenate::Add(Component<float>* iInput)
{
    mInput.push_back(iInput);
    mLength.push_back(iInput->Frame().size);
    mFrame.size += iInput->Frame().size;
    Connect(iInput);
}

bool Tracter::Concatenate::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);
    CacheArea inputArea;

    // Start with the high order inputs as they are likely to be Deltas
    // and it will help to prevent backwards cache reads.
    int arrayPos = mFrame.size;
    for (int i=mInput.size()-1; i>=0; i--)
    {
        if(mInput[i]->Read(inputArea, iIndex) == 0)
            return false;
        arrayPos -= mLength[i];
        assert(arrayPos >= 0);
        float* p = mInput[i]->GetPointer(inputArea.offset);
        for (int j=0; j<mLength[i]; j++)
            oData[arrayPos+j] = p[j];
    }

    return true;
}
