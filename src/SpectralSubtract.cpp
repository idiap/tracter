/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "SpectralSubtract.h"

Tracter::PluginObject* Tracter::SpectralSubtract::GetInput(int iInput)
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

Tracter::SpectralSubtract::SpectralSubtract(
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

    mAlpha = GetEnv("Alpha", 1.0f);
    mBeta = GetEnv("Beta", 0.0f);

    for (int i=0; i<mNInputs; i++)
        MinSize(GetInput(i), 1);
}

bool Tracter::SpectralSubtract::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);
    CacheArea inputArea;
    float* cache = GetPointer(iOffset);

    // Start with the second input, likely to be a cepstral or
    // spectral mean
    if (mInput2->Read(inputArea, iIndex) == 0)
        return false;
    float *p2 = mInput2->GetPointer(inputArea.offset);

    // Now the first input
    if (mInput1->Read(inputArea, iIndex) == 0)
        return false;
    float *p1 = mInput1->GetPointer(inputArea.offset);

    // Do the subtraction
    for (int i=0; i<mArraySize; i++)
    {
        float weighted = p1[i] - mAlpha * p2[i];
        float floored  = p2[i] * mBeta;
        cache[i] = (weighted > floored) ? weighted : floored;
    }

    return true;
}
