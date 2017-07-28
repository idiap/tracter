/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cmath>

#include "Log.h"

/**
 * Calculate log.
 */
Tracter::Log::Log(Component<float>* iInput, const char* iObjectName)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(mInput);
    mFrame.size = iInput->frame().size;
    assert(mFrame.size >= 0);

    mFloor = config("Floor", 1e-8f);
    mLogFloor = logf(mFloor);
    mFloored = 0;
}

Tracter::Log::~Log()
{
    if (mFloored > 0)
        verbose(1, "floored %d values < %e\n", mFloored, mFloor);
}

bool Tracter::Log::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame
    const float* ip = mInput->unaryRead(iIndex);
    if (!ip)
        return false;

    // Copy the frame though a log function
    for (int i=0; i<mFrame.size; i++)
        if (ip[i] > mFloor)
            oData[i] = logf(ip[i]);
        else
        {
            oData[i] = mLogFloor;
            mFloored++;
        }

    // Done
    return true;
}
