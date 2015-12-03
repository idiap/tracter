/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Divide.h"

Tracter::Divide::Divide(
    Component<float>* iInput1,
    Component<float>* iInput2,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput1 = iInput1;
    mInput2 = iInput2;
    Connect(iInput1);
    Connect(iInput2);

    mFrame.size = iInput1->Frame().size;
}

bool Tracter::Divide::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);
    CacheArea inputArea;

    // Start with the second input, likely to be a cepstral mean
    if (mInput2->Read(inputArea, iIndex) == 0)
        return false;
    float *p2 = mInput2->GetPointer(inputArea.offset);

    // Now the first input
    if (mInput1->Read(inputArea, iIndex) == 0)
        return false;
    float *p1 = mInput1->GetPointer(inputArea.offset);

    // Do the division
    for (int i=0; i<mFrame.size; i++)
        oData[i] = p1[i] / p2[i];

    return true;
}
