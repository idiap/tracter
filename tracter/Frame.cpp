/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Frame.h"

Tracter::Frame::Frame(Component<float>* iInput, const char* iObjectName)
{
    objectName(iObjectName);
    mFrame.size = config("Size", 256);
    mFrame.period = config("Period", 80);
    mInput = iInput;

    // Framers look ahead, not back
    connect(mInput, mFrame.size, mFrame.size-1);

    assert(mFrame.size > 0);
    assert(mFrame.period > 0);
}

bool Tracter::Frame::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    IndexType readIndex = iIndex * mFrame.period;
    int got = mInput->read(inputArea, readIndex, mFrame.size);
    if (got < mFrame.size)
        return false;

    // Copy to output
    float* ip = mInput->getPointer();
    for (int i=0; i<inputArea.len[0]; i++)
        oData[i] = ip[inputArea.offset+i];
    for (int i=0; i<inputArea.len[1]; i++)
        oData[inputArea.len[0]+i] = ip[i];

    // Done
    return true;
}
