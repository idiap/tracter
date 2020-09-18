/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Delta.h"

Tracter::Delta::Delta(Component<float>* iInput, const char* iObjectName)
{
    objectName(iObjectName);
    mInput = iInput;
    mFrame.size = iInput->frame().size;
    assert(mFrame.size > 0);

    mTheta = config("Theta", 2);
    assert(mTheta > 0);

    mWindow = mTheta*2 + 1;
    mFeature.resize(mWindow);
    connect(mInput, mWindow, mTheta);

    // Set the weights in advance
    mWeight.resize(mWindow);
    float denom = 0.0f;
    for (int i=1; i<=mTheta; i++)
        denom += 2.0*i*i;
    for (int i=0; i<mWindow; i++)
        mWeight[i] = (float)(i - mTheta) / denom;
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::Delta::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Decide how many frames to request depending on whether need to
    // take into account the edge-effect at the beginning.
    int wanted;
    int readIndex;
    if (iIndex < mTheta)
    {
        wanted = mWindow - (mTheta - iIndex);
        readIndex = 0;
    }
    else
    {
        wanted = mWindow;
        readIndex = iIndex - mTheta;
    }
    int lenGot = mInput->read(inputArea, readIndex, wanted);
    if (lenGot == 0)
        return false;

    // To handle the edges, duplicate frames at the edges so there's
    // always the right number of frames for the regression.
    int feature = 0;
    int offset = inputArea.offset;
    float* p = mInput->getPointer(offset);
    for (int i=0; i<mWindow-wanted; i++)
    {
        mFeature[feature++] = p;
    }

    // Copy pointers as if no edge effect
    for (int i=0; i<lenGot; i++)
    {
        if (i == inputArea.len[0])
            offset = 0;
        mFeature[feature++] = mInput->getPointer(offset++);
    }

    // If we got frames back, but fewer than required, then we need to
    // consider the edge-effect at the end.
    int lastFeature = feature-1;
    for (int i=0; i<wanted-lenGot; i++)
    {
        mFeature[feature++] = mFeature[lastFeature];
    }
    assert(feature == mWindow);

    // The actual calculation
    for (int j=0; j<mFrame.size; j++)
        oData[j] = 0.0f;
    for (int i=0; i<mWindow; i++)
    {
        if (i == mTheta)  // The weight is zero
            continue;
        for (int j=0; j<mFrame.size; j++)
            oData[j] += mFeature[i][j] * mWeight[i];
    }

    return true;
}
