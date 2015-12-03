/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "Noise.h"

Tracter::Noise::Noise(Component<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mInput = iInput;
    mFrame.size = iInput->Frame().size;

    mValid = false;
    mNInit = GetEnv("NInit", 10);
    mEnd = GetEnv("End", 0);
    mSoftReset = GetEnv("SoftReset", 0);
    mWrite = GetEnv("Write", 0);
    if (mEnd)
        // Store everything - we'll read the end first
        Connect(iInput, ReadRange::INFINITE);
    else
        // Store enough for the initialisation
        Connect(iInput, 1, mNInit-1);

    mAccumulator.resize(mFrame.size, 0.0f);
    mNAccumulated = 0;
}

Tracter::Noise::~Noise() throw ()
{
    if (mSoftReset)
        Verbose(1, "accumulated %d samples\n", mNAccumulated);
    if (mWrite)
    {
        Calculate(&mAccumulator.front());  // Overwrite!
        for (int i=0; i<mFrame.size; i++)
            printf("%e\n", mAccumulator[i]);
    }
}

void Tracter::Noise::Reset(bool iPropagate)
{
    // Invalidate the estimate
    mValid = false;

    if (!mSoftReset)
    {
        mNAccumulated = 0;
        mAccumulator.assign(mFrame.size, 0.0f);
    }

    // Call the base class
    CachedComponent<float>::Reset(iPropagate);
}

bool Tracter::Noise::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(mSize == 1);

    if (!mValid)
    {
        // Accumulate over some initial number of samples
        CacheArea ca;
        for (int j=0; j<mNInit; j++)
        {
            if (!mInput->Read(ca, j))
                return false;

            float* input = mInput->GetPointer(ca.offset);
            Accumulate(input);
        }

        if (mEnd)
        {
            // Read to the end
            int frameCount = mNInit;
            while (mInput->Read(ca, frameCount++)) {
                // No operation
            }

            // Accumulate over the final samples
            for (int j=frameCount-mNInit; j<frameCount-1; j++)
            {
                if (!mInput->Read(ca, j))
                {
                    Verbose(1, "out of data\n");
                    return false;
                }

                float* input = mInput->GetPointer(ca.offset);
                Accumulate(input);
            }
        }

        // Divide through to finalise the estimate
        Calculate(oData);
        mValid = true;

        // Print out the noise estimate
        if (sVerbose > 2)
            for (int i=0; i<mFrame.size; i++)
                printf("%e\n", oData[i]);
    }

    // After the initialisation, there is nothing to do.
    // ...until someone wants to get smart with VAD or something
    return true;
}
