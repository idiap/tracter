/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "SpectralSubtract.h"

Tracter::SpectralSubtract::SpectralSubtract(
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

    mAlpha = config("Alpha", 1.0f);
    mBeta = config("Beta", 0.0f);
}

bool Tracter::SpectralSubtract::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);
    CacheArea inputArea;

    // Start with the second input, likely to be a cepstral or
    // spectral mean
    if (mInput2->Read(inputArea, iIndex) == 0)
        return false;
    float *p2 = mInput2->getPointer(inputArea.offset);

    // Now the first input
    if (mInput1->Read(inputArea, iIndex) == 0)
        return false;
    float *p1 = mInput1->getPointer(inputArea.offset);

    // Do the subtraction
    for (int i=0; i<mFrame.size; i++)
    {
        float weighted = p1[i] - mAlpha * p2[i];
        float floored  = p2[i] * mBeta;
        oData[i] = (weighted > floored) ? weighted : floored;
    }

    return true;
}
