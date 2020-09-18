/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include <cstdio>

#include "Modulation.h"

void Tracter::SlidingDFT::setRotation(int iBin, int iNBins)
{
    float a = 2.0f * M_PI * iBin / iNBins;
    float r = cos(a);
    float i = sin(a);
    mRotation = complex(r, i);
}

const Tracter::complex& Tracter::SlidingDFT::transform(float iNew, float iOld)
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
    objectName(iObjectName);
    mInput = iInput;
    connect(iInput);

    mFrame.size = 1;
    assert(iInput->frame().size == 1);

    /* For a 100Hz frame rate and bin 1 = 4Hz, we have nBins = 100/4 =
     * 25 */
    float freq = config("Freq", 4.0f);
    int bin = config("Bin", 1);
    mNBins = (int)(frameRate() / freq + 0.5f);
    mDFT.setRotation(bin, mNBins);
    mLookAhead = mNBins / 2; // Round down
    mLookBehind = mNBins - mLookAhead - 1;
    minSize(mInput, mNBins, mLookAhead);
    mIndex = -1;

    verbose(2, "NBins=%d (-%d+%d)\n", mNBins, mLookBehind, mLookAhead);
}

void Tracter::Modulation::reset(bool iPropagate)
{
    verbose(2, "Reset\n");
    mIndex = -1;
    CachedComponent<float>::reset(iPropagate);
}

bool Tracter::Modulation::unaryFetch(IndexType iIndex, float* oData)
{
    verbose(3, "iIndex %ld\n", iIndex);
    assert(iIndex == mIndex+1);
    mIndex = iIndex;

    if (iIndex == 0)
    {
        /*
         * This is quite tricky.  We need to make the DFT memory look
         * like it's been running continuously up to index -1.  That
         * is, reset, then shift in mLookBehind + 1 + mLookAhead
         * samples.  Below, assume a window of 12+1+12 = 25 frames.
         */
        mDFT.reset();
        SizeType len = mLookAhead;
        const float* p = mInput->contiguousRead(0, len);
        if (!p)
            return false;
        assert(len == mLookAhead); // The cache should be big enough

        /* Prime with the first sample for all the missing ones */
        for (SizeType i=0; i<mLookBehind+1; i++)
            mDFT.transform(p[0], 0.0f);

        /* Prime the rest with the look-ahead; this includes the first
         * sample once more, so there are 14 copies of the first
         * sample */
        for (SizeType i=0; i<mLookAhead; i++)
            mDFT.transform(p[i], 0.0f);
    }

    /* Read the old value - the one just behind the DFT window.  This
     * should be index zero the first 14 times for a 12+1+12 window */
    IndexType loIndex = iIndex > mLookBehind
        ? iIndex - mLookBehind - 1
        : 0;
    const float* oldVal = mInput->unaryRead(loIndex);
    if (!oldVal)
        return false;

    /* Check that the current value is valid */
    if (!mInput->unaryRead(iIndex))
        return false;

    /* Now the new lookahead value.  Read back from the end until it's
     * found.  It'll be the first hit unless near the end */
    const float* newVal;
    for (IndexType hiIndex=iIndex+mLookAhead; hiIndex>=iIndex; hiIndex--)
    {
        newVal = mInput->unaryRead(hiIndex);
        if (newVal)
            break;
    }

    /* Do the transform */
    complex tmp = mDFT.transform(*newVal, *oldVal);
    float filter = abs(tmp);
    filter /= mNBins;
    *oData = filter;
    return true;
}
