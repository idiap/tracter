/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Divide.h"

PluginObject* Divide::GetInput(int iInput)
{
    // Enumerate the inputs
    switch (iInput)
    {
        case 0:
            return mInput1;
        case 1:
            return mInput2;
        default:
            assert(0);
    }

    // Should never get here
    return 0;
}

Divide::Divide(
    Plugin<float>* iInput1,
    Plugin<float>* iInput2,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mArraySize = iInput1->GetArraySize();

    Connect(iInput1);
    Connect(iInput2);

    mInput1 = iInput1;
    mInput2 = iInput2;

    for (int i=0; i<mNInputs; i++)
        MinSize(GetInput(i), 1);
}

bool Divide::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);
    CacheArea inputArea;
    float* cache = GetPointer(iOffset);

    // Start with the second input, likely to be a cepstral mean
    if (mInput2->Read(inputArea, iIndex) == 0)
        return false;
    float *p2 = mInput2->GetPointer(inputArea.offset);

    // Now the first input
    if (mInput1->Read(inputArea, iIndex) == 0)
        return false;
    float *p1 = mInput1->GetPointer(inputArea.offset);

    // Do the division
    for (int i=0; i<mArraySize; i++)
        cache[i] = p1[i] / p2[i];

    return true;
}
