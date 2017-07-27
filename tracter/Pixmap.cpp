/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cmath>
#include <cfloat>

#include "Pixmap.h"

Tracter::Pixmap::Pixmap(Component<float>* iInput, const char* iObjectName)
{
    objectName(iObjectName);
    mInput = iInput;

    mFrame.size = config("FrameSize", iInput->frame().size);
    assert(mFrame.size >= 0);

    // Keep all the input
    connect(iInput, ReadRange::INFINITE);

    mLoIndex = -1;
    mHiIndex = -1;
    mMin = FLT_MAX;
    mMax = -FLT_MAX;

    mLog = config("Log", 1);
    mRange = config("Range", 90);
}

bool Tracter::Pixmap::unaryFetch(IndexType iIndex, float* oData)
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
    float* input  = mInput->getPointer(inputArea.offset);
    for (int i=0; i<mFrame.size; i++)
    {
        oData[i] = input[i];
        if (input[i] > mMax)
            mMax = input[i];
        if (input[i] < mMin)
            mMin = input[i];
    }

    mHiIndex = iIndex;
    return true;
}

void Tracter::Pixmap::write()
{
    // Header
    assert(mLoIndex >= 0);
    assert(mHiIndex >= 0);
    assert(mHiIndex >= mLoIndex);
    assert(mMax > mMin);
    printf("P2\n");
    printf("%d %d\n", mFrame.size, mHiIndex - mLoIndex + 1);
    printf("255\n");

    // Body
    if (mLog && (mMin < 0.0f))
        throw Exception("%s: Minimum value is < 0, can't use log scaling\n",
                        objectName());

    // Scale the available range to 0-255.  Can be floored.
    float min;
    float max;
    float scale;
    float range;
    if (mLog)
    {
        min = log10f(mMin);
        max = log10f(mMax);
        range = std::min(max - min, mRange/10.0f);
        verbose(1, "Range: %.2f dB raw\n", 10.0*(max-min));
        verbose(1, "Range: %.2f dB floored\n", 10.0*(range));
    }
    else
    {
        min = mMin;
        max = mMax;
	range = std::min(max - min, powf(10, mRange/10.0f));
    }
    scale = 255.0f / range;
    min = max - range;

    CacheArea inputArea;
    for (int f=mLoIndex; f<=mHiIndex; f++)
    {
        mInput->Read(inputArea, f);
        assert(inputArea.length() == 1);
        float* p = mInput->getPointer(inputArea.offset);
        for (int i=0; i<mFrame.size; i++)
        {
            int val = (int)std::max(
                ((mLog ? log10f(p[i]) : p[i]) - min) * scale,
                0.0f
            );
            printf(" %d", val);
        }
        printf("\n");
    }
}
