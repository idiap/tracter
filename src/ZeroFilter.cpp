/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ZeroFilter.h"

ZeroFilter::ZeroFilter(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mZero = GetEnv("Zero", 0.97f);
}

void ZeroFilter::MinSize(int iSize, int iReadAhead)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    PluginObject::MinSize(iSize, iReadAhead);

    // We expect the input buffer to be at least the size of each
    // request, then +1 for the possible read-behind
    assert(mInput);
    PluginObject::MinSize(mInput, iSize+1, 0);
}

int ZeroFilter::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    float store = 0.0f;
    float* input = mInput->GetPointer();

    // Do a single read behind the window to initialise the store
    // This makes the code afterwards identical for iIndex == 0 too.
    if (iIndex > 0)
    {
        int one = mInput->Read(inputArea, iIndex-1);
        if (one == 0)
            return 0;
        store = input[inputArea.offset];
    }

    // The usual read and offset initialisation
    int lenGot = mInput->Read(inputArea, iIndex, iOutputArea.Length());
    int rOffset = inputArea.offset;
    int wOffset = iOutputArea.offset;

    // For the edge effect, duplicate the first sample
    if (iIndex == 0)
        store = input[rOffset];

    // Main calculation
    float* output = GetPointer();
    for (int i=0; i<lenGot; i++)
    {
        if (i == inputArea.len[0])
            rOffset = 0;
        if (i == iOutputArea.len[0])
            wOffset = 0;

        output[wOffset++] = input[rOffset] - mZero * store;
        store = input[rOffset++];
    }

    return lenGot;
}
