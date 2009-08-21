/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include <cstdio>

#include "Modulation.h"

void Tracter::SlidingDFT::SetRotation(int iBin, int iNBins)
{
    float a = 2.0f * M_PI * iBin / iNBins;
    float r = cosf(a);
    float i = sinf(a);
    mRotation = complex(r, i);
}

const Tracter::complex& Tracter::SlidingDFT::Transform(float iNew, float iOld)
{
    float tmp = iNew - iOld;
    complex ctmp = mState + tmp;
    mState = mRotation * ctmp;
    return mState;
}

Tracter::Modulation::Modulation(
    Component<float>* iInput, const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(iInput);

    mFrame.size = 1;
    assert(iInput->Frame().size == 1);

    /* For a 100Hz frame rate and bin 1 = 4Hz, we have nBins = 100/4 =
     * 25 */
    float freq = GetEnv("Freq", 4.0f);
    int bin = GetEnv("Bin", 1);
    mNBins = (int)(FrameRate() / freq + 0.5f);
    mDFT.SetRotation(bin, mNBins);
    mLookAhead = mNBins / 2; // Round down
    mLookBehind = mNBins - mLookAhead - 1;
    MinSize(mInput, mNBins, mLookAhead);
    mIndex = -1;

    Verbose(2, "NBins=%d (-%d+%d)\n", mNBins, mLookBehind, mLookAhead);
}

void Tracter::Modulation::Reset(bool iPropagate)
{
    Verbose(2, "Reset\n");
    mIndex = -1;
}

bool Tracter::Modulation::UnaryFetch(IndexType iIndex, float* oData)
{
    Verbose(3, "iIndex %ld\n", iIndex);
    assert(iIndex == mIndex+1);
    mIndex = iIndex;
    CacheArea inputArea;

    float filter = 0.0f;
    float energy = 0.0f;

    if (iIndex == 0)
    {
        /* Reset and prime half the DFT with the first sample */
        mDFT.Reset();
        const float* p = mInput->UnaryRead(iIndex);
        if (!p)
            return false;
        energy = p[0];
        for (int i=0; i<mLookBehind; i++)
            mDFT.Transform(p[0], 0.0f);

        /* Prime the rest of the DFT with the look-ahead. */
        if (!mInput->Read(inputArea, iIndex, mLookAhead+1))
            return false;
        assert(inputArea.len[1] == 0); // The cache should be big enough
        p = mInput->GetPointer(inputArea.offset);
        for (int i=0; i<mLookAhead+1; i++)
            mDFT.Transform(p[i], 0.0f);
    }

    /* Read the old value - the one just behind the DFT window */
    IndexType oldIndex = iIndex > mLookBehind
        ? iIndex - mLookBehind - 1
        : 0;
    if (!mInput->Read(inputArea, oldIndex))
        return false;
    float oldVal = *mInput->GetPointer(inputArea.offset);

    /* Current value */
    if (!mInput->Read(inputArea, iIndex))
        return false;
    energy = *mInput->GetPointer(inputArea.offset);

    /* Now the new lookahead value.  Read back from the end until it's
     * found.  It'll be the first hit unless near the end */
    IndexType in = 0;
    for (in=iIndex+mLookAhead; in>=iIndex; in--)
        if (mInput->Read(inputArea, in))
            break;
    float newVal = *mInput->GetPointer(inputArea.offset);

    /* Do the transform */
    complex tmp = mDFT.Transform(newVal, oldVal);
    filter = abs(tmp);
    filter /= mNBins;
    *oData = filter;
    return true;
}
