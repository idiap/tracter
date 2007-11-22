/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Noise.h"

Noise::Noise(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = iInput->GetArraySize();
    mValid = false;
    mNInit = GetEnv("NInit", 10);
    MinSize(iInput, 1, mNInit-1);
}

bool Noise::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(mSize == 1);

    if (!mValid)
    {
        // Get and zero the noise estimate
        float* output = GetPointer(iOffset);
        for (int i=0; i<mArraySize; i++)
            output[i] = 0.0f;

        // Accumulate over some initial number of samples
        for (int j=0; j<mNInit; j++)
        {
            CacheArea ca;
            if (!mInput->Read(ca, j))
                return false;

            float* input = mInput->GetPointer(ca.offset);
            for (int i=0; i<mArraySize; i++)
                output[i] += input[i];
        }

        // Divide through to finalise the estimate
        for (int i=0; i<mArraySize; i++)
            output[i] /= (float)mNInit;
        mValid = true;
    }

    // After the initialisation, there is nothing to do.
    // ...until someone wants to get smart with VAD or something
    return true;
}
