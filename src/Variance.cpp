/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstring>
#include <cmath>

#include "Variance.h"

Tracter::Variance::Variance(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = iInput->GetArraySize();
    assert(mArraySize >= 0);

    mVarianceType = VARIANCE_ADAPTIVE;

    mBurnIn = GetEnv("BurnIn", 20);

    if (const char* env = GetEnv("Type", "ADAPTIVE"))
    {
        if (strcmp(env, "STATIC") == 0)
            mVarianceType = VARIANCE_STATIC;
    }

    switch (mVarianceType)
    {
    case VARIANCE_STATIC:
        // Set the input buffer to store everything
        PluginObject::MinSize(mInput, -1);
        break;

    case VARIANCE_ADAPTIVE:
        PluginObject::MinSize(mInput, mBurnIn);
        break;

    default:
        assert(0);
    }

    mVariance.resize(mArraySize);
    for (int i=0; i<mArraySize; i++)
        mVariance[i] = 0.0;
    mValid = false;
    SetTimeConstant(GetEnv("TimeConstant", 0.5f));
}

void Tracter::Variance::SetTimeConstant(float iSeconds)
{
    assert(iSeconds > 0);
    float n = iSeconds * mSampleFreq / mSamplePeriod;
    mPole = (n-1.0f) / n;
    mElop = 1.0f - mPole;

    assert(mPole > 0.0f);
    assert(mPole < 1.0f);
    Verbose(1, "Variance: pole is %f\n", mPole);
}

void Tracter::Variance::Reset(bool iPropagate)
{
    // Zero the variance
    for (int i=0; i<mArraySize; i++)
        mVariance[i] = 0.0;
    mValid = false;

    // Call the base class
    UnaryPlugin<float, float>::Reset(iPropagate);
}

bool Tracter::Variance::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    switch (mVarianceType)
    {
    case VARIANCE_STATIC:
        if (!mValid)
            processAll();
        break;

    case VARIANCE_ADAPTIVE:
        if (!adaptFrame(iIndex))
            return false;
        break;

    default:
        assert(0);
    }

    // Copy to output, which is a bit of a waste if the output is only
    // size 1 and there's only one variance.  Maybe there's an
    // optimisation possible.
    float* output = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
        output[i] = sqrtf(mVariance[i]); // Wasteful for static!

    return true;
}

void Tracter::Variance::processAll()
{
    // Calculate variance over whole input range
    int frame = 0;
    CacheArea inputArea;
    while(mInput->Read(inputArea, frame))
    {
        assert(inputArea.Length() == 1);
        float* p = mInput->GetPointer(inputArea.offset);
        for (int i=0; i<mArraySize; i++)
            mVariance[i] += p[i] * p[i];
        frame++;
    }
    if (frame > 0)
        for (int i=0; i<mArraySize; i++)
            mVariance[i] /= frame;
    mValid = true;

#if 0
    printf("Variance got %d frames\n", frame);
    for (int i=0; i<4; i++)
        printf(" %f", mVariance[i]);
#endif
}

bool Tracter::Variance::adaptFrame(IndexType iIndex)
{
    assert(iIndex >= 0);

    CacheArea inputArea;

    if (!mValid)
    {
        // Set the variance using the first mBurnIn frames
        mAdaptStart = iIndex+mBurnIn;
        for (int i=iIndex; i<iIndex+mBurnIn; i++)
        {
            if (mInput->Read(inputArea, i) == 0)
                return false;
            assert(inputArea.Length() == 1);
            float* p = mInput->GetPointer(inputArea.offset);
            for (int j=0; j<mArraySize; j++)
                mVariance[j] += p[j] * p[j];
        }
        for (int j=0; j<mArraySize; j++)
            mVariance[j] /= mBurnIn;
        mValid = true;
        Verbose(1, "Burn in gives %e %e %e %e ...\n",
                mVariance[0], mVariance[1], mVariance[2], mVariance[3]);
    }

    if (iIndex >= mAdaptStart)
    {
        if (mInput->Read(inputArea, iIndex) == 0)
            return false;
        assert(inputArea.Length() == 1);
        float* p = mInput->GetPointer(inputArea.offset);

        // Combine the new observation into the variance
        for (int i=0; i<mArraySize; i++)
            mVariance[i] = mPole * mVariance[i] + mElop * p[i] * p[i];
    }

    return true;
}
