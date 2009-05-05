/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Frame.h"

Tracter::Frame::Frame(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mFrameSize = GetEnv("Size", 256);
    mArraySize = mFrameSize;
    mFramePeriod = GetEnv("Period", 80);
    mSamplePeriod *= mFramePeriod;
    assert(mFrameSize > 0);
    assert(mFramePeriod > 0);

    // Framers look ahead, not back
    PluginObject::MinSize(mInput, mFrameSize, mFrameSize-1);
}

bool Tracter::Frame::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    int readIndex = iIndex * mFramePeriod;
    int got = mInput->Read(inputArea, readIndex, mFrameSize);
    if (got < mFrameSize)
        return false;

    // Copy to output
    float* ip = mInput->GetPointer();
    float* op = GetPointer(iOffset);
    for (int i=0; i<inputArea.len[0]; i++)
        op[i] = ip[inputArea.offset+i];
    for (int i=0; i<inputArea.len[1]; i++)
        op[inputArea.len[0]+i] = ip[i];

    // Done
    return true;
}
