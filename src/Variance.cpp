/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cstring>
#include <cmath>

#include "Variance.h"

Tracter::Variance::Variance(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = iInput->GetArraySize();
    assert(mArraySize >= 0);

    mAdaptStart = 0;
    mVarianceType = VARIANCE_ADAPTIVE;

    mBurnIn = GetEnv("BurnIn", 20);
    mPersistent = GetEnv("Persistent", 0);

    if (const char* env = GetEnv("Type", "ADAPTIVE"))
    {
        if (strcmp(env, "STATIC") == 0)
            mVarianceType = VARIANCE_STATIC;
        else if (strcmp(env, "FIXED") == 0)
            mVarianceType = VARIANCE_FIXED;
    }

    switch (mVarianceType)
    {
    case VARIANCE_STATIC:
        // Set the input buffer to store everything
        PluginObject::MinSize(mInput, -1);
        mValid = false;
        break;

    case VARIANCE_ADAPTIVE:
        PluginObject::MinSize(mInput, std::max(mBurnIn, 1));
        mValid = false;
        break;

    case VARIANCE_FIXED:
        // In fact, the input will never be read
        PluginObject::MinSize(mInput, 1);
        mValid = true;
        break;

    default:
        assert(0);
    }

    // Possibly initialise a prior variance
    const char* priFile = GetEnv("PriorFile", (const char*)0);
    if (priFile)
        Load(mPrior, "<VARIANCE>", priFile);

    // Initialise a target variance either from a file or to unity
    const char* tgtFile = GetEnv("TargetFile", (const char*)0);
    if (tgtFile)
        Load(mTarget, "<VARSCALE>", tgtFile);
    else
        mTarget.assign(mArraySize, 1.0);

    // Our running variance is initialised to the prior if there,
    // otherwise the target
    if (mPrior.size())
        mVariance.assign(mPrior.begin(), mPrior.end());
    else
        mVariance.assign(mTarget.begin(), mTarget.end());

    // Time constant
    SetTimeConstant(GetEnv("TimeConstant", 1.0f));
}

/**
 * Convert a time in seconds to a time constant.  This is a pole in a
 * single pole filter (sometime called a forgetting factor) with value
 * (n-1)/n, where n is in frames.  I've seen it written as (n-1)/(n+1)
 * too, but can't find a persuasive derivation.
 */
void Tracter::Variance::SetTimeConstant(float iSeconds)
{
    assert(iSeconds > 0);
    float n = iSeconds * mSampleFreq / mSamplePeriod;
    mPole = (n-1.0f) / n;
    mElop = 1.0f - mPole;

    assert(mPole > 0.0f);
    assert(mPole < 1.0f);
    Verbose(1, "Pole is %f\n", mPole);
}

void Tracter::Variance::Reset(bool iPropagate)
{
    // Reset the variance to the target
    if (!mPersistent || (mVarianceType != VARIANCE_ADAPTIVE))
    {
        if (mPrior.size())
            mVariance.assign(mPrior.begin(), mPrior.end());
        else
            mVariance.assign(mTarget.begin(), mTarget.end());
        if (mVarianceType != VARIANCE_FIXED)
            mValid = false;
    }

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

    case VARIANCE_FIXED:
        // Do nothing
        break;

    default:
        assert(0);
    }

    float* output = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
        output[i] = sqrtf(mVariance[i] / mTarget[i]);

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

    if (mBurnIn && !mValid)
    {
        // Set the variance using the first mBurnIn frames
        mAdaptStart = iIndex+mBurnIn;
        mVariance.assign(mArraySize, 0.0f);
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

void Tracter::Variance::Load(
    std::vector<float>& iVariance, const char* iToken, const char* iFileName
)
{
    Verbose(1, "Loading %s from %s\n", iToken, iFileName);
    FILE* fp = fopen(iFileName, "r");
    if (!fp)
        throw Exception("Failed to open file %s", iFileName);

    char tmpStr[20];
    int tmpInt = 0;
    while (!tmpInt)
    {
        if (feof(fp))
            throw Exception("Failed to find %s in %s", iToken, iFileName);
        if (fscanf(fp, "%s", tmpStr) == 1)
            if (strncmp(iToken, tmpStr, 20) == 0)
                if (fscanf(fp, "%d", &tmpInt) != 1)
                    throw Exception("Failed to read vector size");
    }
    if (tmpInt != mArraySize)
        throw Exception("Vector size %d != array size %d", tmpInt, mArraySize);
    iVariance.resize(mArraySize);
    for (int i=0; i<mArraySize; i++)
        if (fscanf(fp, "%f", &iVariance[i]) != 1)
            throw Exception("failed to read element %d", i);
#if 0
    for (int i=0; i<mArraySize; i++)
        printf("%f\n", iVariance[i]);
#endif
}
