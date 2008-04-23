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
    mEnd = GetEnv("End", 0);
    if (mEnd)
        // Store everything - we'll read the end first
        MinSize(iInput, -1);
    else
        // Store enough for the initialisation
        MinSize(iInput, 1, mNInit-1);
}

void Noise::Reset(bool iPropagate)
{
    // Invalidate the estimate
    mValid = false;

    // Call the base class
    UnaryPlugin<float, float>::Reset(iPropagate);
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
        CacheArea ca;
        for (int j=0; j<mNInit; j++)
        {
            if (!mInput->Read(ca, j))
                return false;

            float* input = mInput->GetPointer(ca.offset);
            for (int i=0; i<mArraySize; i++)
                output[i] += input[i];
        }

        if (mEnd)
        {
            // Read to the end
            int frameCount = mNInit;
            while (mInput->Read(ca, frameCount++)) {
                //printf("frame: %d\n", frameCount);
            }

            //printf("frameCount is %d", frameCount);

            // Accumulate over the final samples
            for (int j=frameCount-mNInit; j<frameCount-1; j++)
            {
                //printf("j: %d\n", j);
                if (!mInput->Read(ca, j))
                {
                    if (Tracter::sVerbose > 0)
                        printf("Noise: out of data\n");
                    return false;
                }

                float* input = mInput->GetPointer(ca.offset);
                for (int i=0; i<mArraySize; i++)
                    output[i] = Accumulate(input[i], output[i]);
            }
        }

        // Divide through to finalise the estimate
        for (int i=0; i<mArraySize; i++)
            output[i] = Calculate(output[i], mEnd ? mNInit*2 : mNInit);
        mValid = true;

        // Print out the noise estimate
        if (Tracter::sVerbose > 2)
            for (int i=0; i<mArraySize; i++)
                printf("%e\n", output[i]);
    }

    // After the initialisation, there is nothing to do.
    // ...until someone wants to get smart with VAD or something
    return true;
}
