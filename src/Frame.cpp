/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Frame.h"

Tracter::Frame::Frame(Component<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mFrame.size = GetEnv("Size", 256);
    mFrame.period = GetEnv("Period", 80);
    mInput = iInput;

    // Framers look ahead, not back
    Connect(mInput, mFrame.size, mFrame.size-1);

    assert(mFrame.size > 0);
    assert(mFrame.period > 0);
}

bool Tracter::Frame::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    int readIndex = iIndex * mFrame.period;
    int got = mInput->Read(inputArea, readIndex, mFrame.size);
    if (got < mFrame.size)
        return false;

    // Copy to output
    float* ip = mInput->GetPointer();
    for (int i=0; i<inputArea.len[0]; i++)
        oData[i] = ip[inputArea.offset+i];
    for (int i=0; i<inputArea.len[1]; i++)
        oData[inputArea.len[0]+i] = ip[i];

    // Done
    return true;
}
