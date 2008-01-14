/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "math.h"
#include "float.h"

#include "Pixmap.h"

Pixmap::Pixmap(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = iInput->GetArraySize();
    assert(mArraySize >= 0);

    // Keep all the input
    PluginObject::MinSize(iInput, -1);

    mLoIndex = -1;
    mHiIndex = -1;
    mMin = FLT_MAX;
    mMax = -FLT_MAX;

    mLog = GetEnv("Log", 1);
}

bool Pixmap::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    if (mLoIndex < 0)
        mLoIndex = iIndex;

    CacheArea inputArea;
    if (mInput->Read(inputArea, iIndex) != 1)
    {
        write();
        return false;
    }

    // Copy input to output with limits check
    float* input  = mInput->GetPointer(inputArea.offset);
    float* output = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
    {
        output[i] = input[i];
        if (input[i] > mMax)
            mMax = input[i];
        if (input[i] < mMin)
            mMin = input[i];
    }

    mHiIndex = iIndex;
    return true;
}

void Pixmap::write()
{
    // Header
    assert(mLoIndex >= 0);
    assert(mHiIndex >= 0);
    assert(mHiIndex >= mLoIndex);
    assert(mMax > mMin);
    printf("P2\n");
    printf("%d %d\n", mArraySize, mHiIndex - mLoIndex + 1);
    printf("255\n");

    // Body
    if (mLog && (mMin < 0.0f))
    {
        printf("%s: Minimum value is < 0, can't use log scaling\n",
               mObjectName);
        exit(EXIT_FAILURE);
    }

    float min;
    float scale;
    if (mLog)
    {
        min = logf(mMin);
        scale = 255.0f / (logf(mMax) - logf(mMin));
    }
    else
    {
        min = mMin;
        scale = 255.0f / (mMax - mMin);
    }
    CacheArea inputArea;
    for (int f=mLoIndex; f<=mHiIndex; f++)
    {
        mInput->Read(inputArea, f);
        assert(inputArea.Length() == 1);
        float* p = mInput->GetPointer(inputArea.offset);
        for (int i=0; i<mArraySize; i++)
            printf(" %d", (int)(((mLog ? logf(p[i]) : p[i]) - min) * scale));
        printf("\n");
    }
}
