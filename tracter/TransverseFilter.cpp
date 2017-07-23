/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cmath>

#include "TransverseFilter.h"

/**
 * Run a filter across the frame.  Right now it's a square one.
 */
Tracter::TransverseFilter::TransverseFilter(
    Component<float>* iInput, const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    Connect(mInput);
    mFrame.size = iInput->Frame().size;
    assert(mFrame.size >= 0);

    mRadius = GetEnv("Radius", 1);
    if (mRadius < 0)
        throw Exception("Radius can't be negative");
}

bool Tracter::TransverseFilter::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame
    const float* ip = mInput->UnaryRead(iIndex);
    if (!ip)
        return false;

    // Filter (non-optimal!)
    for (int i=0; i<mFrame.size; i++)
    {
        float sum = ip[i];
        for (int j=-mRadius; j<0; j++)
            sum += ip[std::max(i+j, 0)];
        for (int j=1; j<=mRadius; j++)
            sum += ip[std::min(i+j, mFrame.size-1)];
        oData[i] = sum / (mRadius + mRadius + 1);
    }

    // Done
    return true;
}
