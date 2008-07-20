/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstring>

#include "Mean.h"

Tracter::Mean::Mean(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = iInput->GetArraySize();
    assert(mArraySize >= 0);

    mMeanType = MEAN_ADAPTIVE;

    if (const char* env = GetEnv("Type", "ADAPTIVE"))
    {
        if (strcmp(env, "STATIC") == 0)
            mMeanType = MEAN_STATIC;
    }

    switch (mMeanType)
    {
    case MEAN_STATIC:
        // Set the input buffer to store everything
        PluginObject::MinSize(mInput, -1);
        break;

    case MEAN_ADAPTIVE:
        PluginObject::MinSize(mInput, 1);
        break;

    default:
        assert(0);
    }

    mMean.resize(mArraySize);
    for (int i=0; i<mArraySize; i++)
        mMean[i] = 0.0;
    mValid = false;
    SetTimeConstant(GetEnv("TimeConstant", 0.5f));
}

void Tracter::Mean::SetTimeConstant(float iSeconds)
{
    assert(iSeconds > 0);
    float n = iSeconds * mSampleFreq / mSamplePeriod;
    mPole = (n-1.0f) / n;
    mElop = 1.0f - mPole;

    assert(mPole > 0.0f);
    assert(mPole < 1.0f);
    if (Tracter::sVerbose)
        printf("Mean: pole is %f\n", mPole);
}

void Tracter::Mean::Reset(bool iPropagate)
{
    // Zero the mean
    for (int i=0; i<mArraySize; i++)
        mMean[i] = 0.0;
    mValid = false;

    // Call the base class
    UnaryPlugin<float, float>::Reset(iPropagate);
}

bool Tracter::Mean::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    switch (mMeanType)
    {
    case MEAN_STATIC:
        if (!mValid)
            processAll();
        break;

    case MEAN_ADAPTIVE:
        if (!adaptFrame(iIndex))
            return false;
        break;

    default:
        assert(0);
    }

    // Copy to output, which is a bit of a waste if the output is only
    // size 1 and there's only one mean.  Maybe there's an
    // optimisation possible.
    float* output = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
        output[i] = mMean[i];

    return true;
}

void Tracter::Mean::processAll()
{
    // Calculate mean over whole input range
    int frame = 0;
    CacheArea inputArea;
    while(mInput->Read(inputArea, frame))
    {
        assert(inputArea.Length() == 1);
        float* p = mInput->GetPointer(inputArea.offset);
        for (int i=0; i<mArraySize; i++)
            mMean[i] += p[i];
        frame++;
    }
    if (frame > 0)
        for (int i=0; i<mArraySize; i++)
            mMean[i] /= frame;
    mValid = true;

#if 0
    printf("Mean got %d frames\n", frame);
    for (int i=0; i<4; i++)
        printf(" %f", mMean[i]);
#endif
}

bool Tracter::Mean::adaptFrame(IndexType iIndex)
{
    assert(iIndex >= 0);

    CacheArea inputArea;
    if (mInput->Read(inputArea, iIndex) == 0)
        return false;
    assert(inputArea.Length() == 1);
    float* p = mInput->GetPointer(inputArea.offset);
    if (mValid)
    {
        // Combine the new observation into the mean
        for (int i=0; i<mArraySize; i++)
            mMean[i] = mPole * mMean[i] + mElop * p[i];
    }
    else
    {
        // Reset the mean to be the observation itself, but divide by
        // 2 so that the final CMS result is not zero.  That zero
        // vector at the beginning causes problems in aurora2,
        // probably becuase the first state of the silence model gets
        // a much smaller variance.
        for (int i=0; i<mArraySize; i++)
            mMean[i] = p[i] / 2.0f;
        mValid = true;
    }

    return true;
}
