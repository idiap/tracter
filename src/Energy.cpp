/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Energy.h"

Tracter::Energy::Energy(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mFrameSize = GetEnv("FrameSize", 256);
    mFramePeriod = GetEnv("FramePeriod", 80);
    mSamplePeriod *= mFramePeriod;
    assert(mFrameSize > 0);
    assert(mFramePeriod > 0);

    PluginObject::MinSize(mInput, mFrameSize);
}

bool Tracter::Energy::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    int readIndex = iIndex * mFramePeriod;
    int got = mInput->Read(inputArea, readIndex, mFrameSize);
    if (got < mFrameSize)
        return false;

    // Calculate energy
    float* p = mInput->GetPointer();
    float* energy = GetPointer(iOffset);
    *energy = 0.0f;
    for (int i=0; i<inputArea.len[0]; i++)
        *energy += p[inputArea.offset+i] * p[inputArea.offset+i];
    for (int i=0; i<inputArea.len[1]; i++)
        *energy += p[i] * p[i];

    // Done
    return true;
}
