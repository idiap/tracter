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
    objectName(iObjectName);
    mInput1 = iInput1;
    mInput2 = iInput2;
    connect(iInput1);
    connect(iInput2);

    mFrame.size = iInput1->frame().size;
}

bool Tracter::Divide::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);
    CacheArea inputArea;

    // Start with the second input, likely to be a cepstral mean
    if (mInput2->read(inputArea, iIndex) == 0)
        return false;
    float *p2 = mInput2->getPointer(inputArea.offset);

    // Now the first input
    if (mInput1->read(inputArea, iIndex) == 0)
        return false;
    float *p1 = mInput1->getPointer(inputArea.offset);

    // Do the division
    for (int i=0; i<mFrame.size; i++)
        oData[i] = p1[i] / p2[i];

    return true;
}
