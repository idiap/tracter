/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <algorithm>
#include "LowEnergyEnvelope.h"

Tracter::LowEnergyEnvelope::LowEnergyEnvelope(
    Component<float>* iInput, const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    mFrame.size = iInput->Frame().size;

    float gamma = GetEnv("Gamma", 0.2f);
    float timeWindow = GetEnv("TimeWindow", 1.0f);
    mNWindow = (int)(timeWindow * FrameRate());
    mNGamma  = (int)(gamma * mNWindow);
    mCorrection = GetEnv("Correction", 1.0f / (1.5f * gamma) / (1.5f * gamma));

    // Set up the sorting arrays
    mTmp.resize(mFrame.size);
    for (int i=0; i<mFrame.size; i++)
        mTmp[i].resize(mNWindow);

    // Store enough for the initialisation
    Connect(mInput, mNWindow, mNGamma-1);
}

bool Tracter::LowEnergyEnvelope::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Say the window is 50 frames.  If gamma is 0.2 then we're
    // looking for 10 noise frames.  If we only read 40 frames, then
    // gamma will give us only 8 noise frames.  So look for a minimum
    // of 10 frames.  The first strategy here is try to read ahead 10
    // frames and 40 back.  We'll get at least 10 frames at the
    // beginning, at least 40 at the end, and 50 otherwise.  Always
    // use the minimum 10 frames.
    //
    // The second strategy is to read ahead half the window length,
    // which is more symmetric but has a longer lag.
#if 0
    // Lookahead 10
    int maxIndex = iIndex + mNGamma - 1;
    int minIndex = iIndex - mNWindow + mNGamma;
#else
    // Lookahead window/2
    int wb2 = mNWindow / 2;
    int maxIndex = iIndex + wb2 - 1;
    int minIndex = iIndex - wb2;
#endif
    if (minIndex < 0)
        minIndex = 0;

    // Read the window first, but check that iIndex is available too
    CacheArea inputArea;
    CacheArea tmp;
    if (!mInput->Read(inputArea, minIndex, maxIndex-minIndex+1))
        return false;
    if (!mInput->Read(tmp, iIndex))
        return false;

    // Read the input window into local vectors
    CacheIterator<float> iter(mInput, inputArea);
    for (int i=0; i<inputArea.Length(); i++, ++iter)
        for (int j=0; j<mFrame.size; j++)
            mTmp[j][i] = iter[j];

    // Sort to get the smallest elements first, average and write out
    for (int j=0; j<mFrame.size; j++)
    {
        std::partial_sort(
            mTmp[j].begin(),
            mTmp[j].begin()+mNGamma,
            mTmp[j].begin()+inputArea.Length()
        );
        float sum = 0.0f;
        for (int g=0; g<mNGamma; g++)
            sum += mTmp[j][g];
        oData[j] = sum / mNGamma * mCorrection;
    }

    // If we get here it was successful
    return true;
}
