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
    objectName(iObjectName);
    mInput = iInput;
    mFrame.size = iInput->frame().size;

    mValid = false;
    mNInit = config("NInit", 10);
    mEnd = config("End", 0);
    mSoftReset = config("SoftReset", 0);
    mWrite = config("Write", 0);
    if (mEnd)
        // Store everything - we'll read the end first
        connect(iInput, ReadRange::INFINITE);
    else
        // Store enough for the initialisation
        connect(iInput, 1, mNInit-1);

    mAccumulator.resize(mFrame.size, 0.0f);
    mNAccumulated = 0;
}

Tracter::Noise::~Noise() throw ()
{
    if (mSoftReset)
        verbose(1, "accumulated %d samples\n", mNAccumulated);
    if (mWrite)
    {
        Calculate(&mAccumulator.front());  // Overwrite!
        for (int i=0; i<mFrame.size; i++)
            printf("%e\n", mAccumulator[i]);
    }
}

void Tracter::Noise::reset(bool iPropagate)
{
    // Invalidate the estimate
    mValid = false;

    if (!mSoftReset)
    {
        mNAccumulated = 0;
        mAccumulator.assign(mFrame.size, 0.0f);
    }

    // Call the base class
    CachedComponent<float>::reset(iPropagate);
}

bool Tracter::Noise::unaryFetch(IndexType iIndex, float* oData)
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

            float* input = mInput->getPointer(ca.offset);
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
                    verbose(1, "out of data\n");
                    return false;
                }

                float* input = mInput->getPointer(ca.offset);
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
