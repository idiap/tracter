/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstring>
#include <cstdio>

#include "Mean.h"

Tracter::Mean::Mean(Component<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mInput = iInput;

    mFrame.size = iInput->Frame().size;
    assert(mFrame.size >= 0);

    mMeanType = MEAN_ADAPTIVE;

    if (const char* env = GetEnv("Type", "ADAPTIVE"))
    {
        if (strcmp(env, "STATIC") == 0)
            mMeanType = MEAN_STATIC;
        else if (strcmp(env, "FIXED") == 0)
            mMeanType = MEAN_FIXED;
    }

    mPersistent = GetEnv("Persistent", 0);

    switch (mMeanType)
    {
    case MEAN_STATIC:
        // Set the input buffer to store everything
        Connect(mInput, ReadRange::INFINITE);
        mValid = false;
        break;

    case MEAN_ADAPTIVE:
        Connect(mInput, 1);
        mValid = false;
        break;

    case MEAN_FIXED:
        // In fact, the input will never be read
        Connect(mInput, 1);
        mValid = true;
        break;

    default:
        assert(0);
    }

    // Initialise a prior variance from file or to zero
    const char* priFile = GetEnv("PriorFile", (const char*)0);
    if (priFile)
        Load(mPrior, "<MEAN>", priFile);
    else
        mPrior.assign(mFrame.size, 0.0f);

    // Initialise the mean to the prior
    mMean.assign(mPrior.begin(), mPrior.end());

    // Time constant
    SetTimeConstant(GetEnv("TimeConstant", 0.5f));
}

void Tracter::Mean::SetTimeConstant(float iSeconds)
{
    assert(iSeconds > 0);
    float n = SecondsToFrames(iSeconds);
    mPole = (n-1.0f) / n;
    mElop = 1.0f - mPole;

    assert(mPole > 0.0f);
    assert(mPole < 1.0f);
    Verbose(1, "Mean: pole is %f\n", mPole);
}

void Tracter::Mean::Reset(bool iPropagate)
{
    // Zero the mean
    if (!mPersistent || (mMeanType != MEAN_ADAPTIVE))
    {
        mMean.assign(mPrior.begin(), mPrior.end());
        if (mMeanType != MEAN_FIXED)
            mValid = false;
    }

    // Call the base class
    CachedComponent<float>::Reset(iPropagate);
}

bool Tracter::Mean::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    switch (mMeanType)
    {
    case MEAN_STATIC:
        if (!mValid)
        {
            Verbose(1, "Reading whole stream at %ld\n", iIndex);
            processAll();
            Verbose(1, "Read.\n");
        }
        break;

    case MEAN_ADAPTIVE:
        if (!adaptFrame(iIndex))
            return false;
        break;

    case MEAN_FIXED:
        // Do nothing
        break;

    default:
        assert(0);
    }

    // Copy to output, which is a bit of a waste if the output is only
    // size 1 and there's only one mean.  Maybe there's an
    // optimisation possible.
    for (int i=0; i<mFrame.size; i++)
        oData[i] = mMean[i];

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
        for (int i=0; i<mFrame.size; i++)
            mMean[i] += p[i];
        frame++;
    }
    if (frame > 0)
        for (int i=0; i<mFrame.size; i++)
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
        for (int i=0; i<mFrame.size; i++)
            mMean[i] = mPole * mMean[i] + mElop * p[i];
    }
    else
    {
        // Reset the mean to be the observation itself, but divide by
        // 2 so that the final CMS result is not zero.  That zero
        // vector at the beginning causes problems in aurora2,
        // probably becuase the first state of the silence model gets
        // a much smaller variance.
        for (int i=0; i<mFrame.size; i++)
            mMean[i] = p[i] / 2.0f;
        mValid = true;
    }

    return true;
}

void Tracter::Mean::Load(
    std::vector<float>& iVector, const char* iToken, const char* iFileName
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
    if (tmpInt != mFrame.size)
        throw Exception("Vector size %d != array size %d", tmpInt, mFrame.size);
    iVector.resize(mFrame.size);
    for (int i=0; i<mFrame.size; i++)
        if (fscanf(fp, "%f", &iVector[i]) != 1)
            throw Exception("failed to read element %d", i);
#if 0
    for (int i=0; i<mFrame.size; i++)
        printf("%f\n", iVector[i]);
#endif
}

