#include "Delta.h"

/**
 * MinSize is important as deltas can be chained together.  An
 * acceleration window of 5 requires a delta window of 9 (2 either
 * side of 5).  Um, maybe it's only 7 or 8 if you do the reading
 * cleverly.  Anyway...
 */
void Delta::MinSize(int iSize)
{
    // First resize this plugin's cache
    assert(iSize > 0);
    PluginObject::MinSize(iSize);

    // Then pass on the resize to the the input plugin
    assert(mInput);
    PluginObject::MinSize(mInput, iSize + 2*mTheta);
}

PluginObject* Delta::GetInput(int iInput)
{
    assert(iInput == 0);
    return mInput;
}

Delta::Delta(Plugin<float>* iInput, int iArraySize, int iTheta)
    : CachedPlugin<float>(iArraySize)
{
    assert(iInput);
    mInput = iInput;
    mNInputs++;

    assert(iArraySize > 0);
    mArraySize = iArraySize;

    assert(iTheta > 0);
    mTheta = iTheta;
    mWindow = iTheta*2 + 1;
    PluginObject::MinSize(mInput, mWindow);
    mFeature.resize(mWindow);

    // Set the weights in advance
    mWeight.resize(mWindow);
    float denom = 0.0f;
    for (int i=1; i<=mTheta; i++)
        denom += 2.0*i*i;
    for (int i=0; i<mWindow; i++)
        mWeight[i] = (float)(i - iTheta) / denom;
}

//
// This is the calculation for one frame.  Pretty trivial, but the
// edge effects make the code quite long.
//
bool Delta::ProcessFrame(IndexType iIndex, int iOffset)
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
    int lenGot = mInput->Read(inputArea, readIndex, wanted);
    if (lenGot == 0)
        return false;

    // To handle the edges, duplicate frames at the edges so there's
    // always the right number of frames for the regression.
    int feature = 0;
    int offset = inputArea.offset;
    float* p = mInput->GetPointer(offset);
    for (int i=0; i<mWindow-wanted; i++)
    {
        mFeature[feature++] = p;
    }

    // Copy pointers as if no edge effect
    for (int i=0; i<lenGot; i++)
    {
        if (i == inputArea.len[0])
            offset = 0;
        mFeature[feature++] = mInput->GetPointer(offset++);
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
    float* cache = GetPointer(iOffset);
    for (int j=0; j<mArraySize; j++)
        cache[j] = 0.0f;
    for (int i=0; i<mWindow; i++)
    {
        if (i == mTheta)  // The weight is zero
            continue;
        for (int j=0; j<mArraySize; j++)
            cache[j] += mFeature[i][j] * mWeight[i];
    }

    return true;
}
