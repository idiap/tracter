/*
 * Copyright 2009 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <limits>

#include "Minima.h"

Tracter::SlidingWindow::SlidingWindow(int windowSize, int windowOffset)
    : windowSize(windowSize)
{
    dataSize = std::numeric_limits<IndexType>::max();
    setWindowOffset(windowOffset);
}

void Tracter::SlidingWindow::setWindowOffset(int windowOffset) {
    this->windowOffset = windowOffset;
}

void Tracter::SlidingWindow::reset() {
    dataSize = std::numeric_limits<IndexType>::max();
    setWindowOffset(-windowSize);
}

void Tracter::SlidingWindow::dump(FILE *out) {
    fprintf(out, "windowOffset %lld [", windowOffset);
    const char *sep = "";
    const IndexType windowStart = windowOffset > 0 ? windowOffset : 0;
    const IndexType windowEnd = windowOffset + windowSize > dataSize ? dataSize : windowOffset + windowSize;
    for(IndexType index = windowStart; index<windowEnd; index++) {
        fprintf(out, "%s%g", sep, get(index));
        sep = ",";
    }
    fprintf(out, "]");
}


Tracter::MinimaWindow::MinimaWindow(
    int window, int nMins
):
    SlidingWindow(window, 0),
    mins(new IndexType[nMins]),
    minsSize(nMins)
{
    reset();
}

Tracter::MinimaWindow::~MinimaWindow()
{
    delete [] mins;
}

void Tracter::MinimaWindow::reset()
{
    SlidingWindow::reset();
    minCount    = 0;
    minSum      = 0.0;
    maxIndex    = -1;
}

void Tracter::MinimaWindow::shrinkLeft()
{
    if(minCount == 0 || windowOffset <= mins[0]) return;    // nothing to do (80%)
    minCount--;
    minSum -= get(mins[0]);
    for(int i = 0; i<minCount; i++) {           // remove mins 0, by shuffling others left...
        mins[i] = mins[i+1];
    }
    if(--maxIndex < 0) determineMaximum();          // lost maximum, re-determine (20% => 4%)
}

void Tracter::MinimaWindow::growRight()
{
    const IndexType   index   = windowOffset + windowSize - 1;

//    if (mDataIndex == 16)
//    {
//        printf("growRight: %d %e %e %ld\n", index, maxValue, minSum, dataSize);
//    }

    if(index >= dataSize)
    {
        // If mins were shifted out, we need to re-scan
        if (minCount < minsSize)
            scanMinimum();
        return;
    }
                                            //  %
    const float  sample  = get(index);                              //  100

    if(minCount < minsSize) {               // room for a little one (20%)      //  100
        if(maxIndex == -1 || sample <= maxValue) {  // small, so add on end (20% => 4%) //   20
            if(maxIndex == -1 || sample == maxValue) {
                maxIndex = minCount;
                maxValue = sample;
            }
            mins[minCount++] = index;                           //    4
            minSum += sample;                               //    4
        } else {                    // big, look for new minimum (80% => 16%)
            scanMinimum();                                  //   16
        }
        return;                                         //   20
    }
    if(sample > maxValue)                                       //   80
        return;                     // big, nothing to do (80% => 64%)  //   64

    // symall, squeeze out old maximum... (20% => 16%) (22.6%)                   //   16
    for(int i = maxIndex; i<minsSize-1; i++) {      // shuffle down...          //  160
        mins[i] = mins[i+1];                                    //  160
    }                                               //  160

    // append...
    mins[minsSize-1] = index;                                   //   16

    // The order here is important; don't use an intermediate float
    minSum -= maxValue;
    minSum += sample;                                    //   16

    // re-determine maximum...
    determineMaximum();                                     //   16

//    if (maxValue > minSum)
//    {
//        printf("maxValue %e > minSum %e  di %d\n", maxValue, minSum, mDataIndex);
//        assert(0);
//    }

    // It can tip just below zero, but too much is wrong
    assert(minSum >= -1e-8);
    minSum = std::max(minSum, 0.0);
}

void Tracter::MinimaWindow::determineMaximum()
{
    maxIndex = 0;
    maxValue = get(mins[0]);
    IndexType *r = &mins[0];
    for(int i = 1; i<minCount; i++) {
        if(get(*++r) > maxValue) {
            maxIndex = i;
            maxValue = get(*r);
        }
    }
}

void Tracter::MinimaWindow::scanMinimum()
{             // find new minimum in window, we don't already have
    static const float floatMax = std::numeric_limits<float>::max();

    float minValue = floatMax;
    IndexType minIndex = -1;
    int minIndexToSkip = 0;                 // current minIndex we should not consider 
    IndexType minToSkip = mins[minIndexToSkip];           // current min we should not consider 

    const IndexType windowStart = windowOffset > 0 ? windowOffset : 0;
    const IndexType windowEnd = windowOffset + windowSize > dataSize ? dataSize : windowOffset + windowSize;
    //const int windowEnd = windowOffset+windowSize;

    for(IndexType i = windowStart; i<windowEnd; i++) {

        if(minIndexToSkip<minCount && i == minToSkip) { // already have it?
            minToSkip = mins[++minIndexToSkip]; // skip over element
            continue;
        }

        float value = get(i);

        if(value < minValue) {              // smallest so far...
            minValue = value;           // remember this one
            minIndex = i;
        }
    }

    if(minIndex == -1) return;

    // insert new value at correct index position.... doh!
    int i;
    for(i = minCount-1; i>=0 && minIndex<mins[i]; i--) {    // shuffle up...
        mins[i+1] = mins[i];
    }
    // insert here...
    maxIndex = i+1;
    maxValue = get(minIndex);
    mins[maxIndex] = minIndex;
    minSum += maxValue;
    minCount++;
}

void Tracter::MinimaWindow::dump(FILE *out) {
    SlidingWindow::dump(out);
    fprintf(out, "mins: [");
    const char *sep = "";
    for(int i = 0; i<minCount; i++) {
        fprintf(out, "%s%e", sep, get(mins[i]));
        sep = ",";
    }
    fprintf(out, "]");
}

void Tracter::SlidingWindow::setData(
    Component<float>* iInput, int iDataIndex,
    IndexType iMinIndex, CacheArea iCacheArea
)
{
    assert(iInput);
    mInput = iInput;
    mDataIndex = iDataIndex;
    mMinIndex = iMinIndex;
    mCacheArea = iCacheArea;
}


float Tracter::SlidingWindow::get(IndexType iIndex)
{
    assert(iIndex >= mMinIndex);
    assert(iIndex < mMinIndex + mCacheArea.Length());
    int offset = (int)(iIndex - mMinIndex);
    float* data = mInput->GetPointer(
        (offset < mCacheArea.len[0])
        ? mCacheArea.offset + offset
        : offset - mCacheArea.len[0]
    );
    return data[mDataIndex];
}


Tracter::Minima::Minima(Component<float>* iInput, const char* iObjectName)
{
    objectName(iObjectName);
    mInput = iInput;
    Connect(mInput);

    mFrame.size = mInput->Frame().size;
    if (mFrame.size == 0)
        mFrame.size = 1;

    mNWindow = SecondsToFrames( GetEnv("WindowTime", 1.0f) );
    mNAhead = mNWindow/2;
    MinSize(mInput, mNWindow+1, mNAhead);

    float gamma = GetEnv("Gamma", 0.2f);
    int nGamma  = (int)(gamma * mNWindow);

    mWindow.resize(mFrame.size);
    for (int i=0; i<mFrame.size; i++)
        mWindow[i] = new MinimaWindow(mNWindow, nGamma);

    mCorrection = GetEnv("Correction", 1.0f / (1.5f * gamma) / (1.5f * gamma));

    mLastIndex = -1;
    Verbose(1, "Window %d, %d ahead\n", mNWindow, mNAhead);
}

Tracter::Minima::~Minima() throw ()
{
    for (int i=0; i<mFrame.size; i++)
        delete mWindow[i];
}

/**
 * This is just a wrapper for the unaryFetch().  It enforces the
 * sequential constraint by read samples that have been skipped.  This
 * sort of thing ought to be in the framework rather than here.
 */
bool Tracter::Minima::UnaryFetch(IndexType iIndex, float* oData)
{
    IndexType index = mLastIndex < 0 ? iIndex : mLastIndex + 1;
    for (IndexType i = index; i<=iIndex; i++)
    {
        if (!unaryFetch(i, oData))
            return false;
    }
    return true;
}

/**
 * The real unary fetch.
 */
bool Tracter::Minima::unaryFetch(IndexType iIndex, float* oData)
{
    // Read the window, plus the one datum before it that we're removing
    CacheArea ca;
    IndexType getIndex = std::max((IndexType)0, iIndex + mNAhead - mNWindow);
    int nGet = (int)std::min((IndexType)mNWindow + 1, iIndex + mNAhead + 1);
    int nGot = mInput->Read(ca, getIndex, nGet);
    Verbose(4, "Got %d of %d from %ld\n", nGot, nGet, getIndex);

    // This means iIndex is off the end - we're done
    if (nGet - nGot > mNAhead)
        return false;

    // We're approaching the end; tell the windower
    if ((nGot < nGet) && mEndOfData < 0)
    {
        IndexType dataSize = getIndex + nGot;
        mEndOfData = getIndex + nGot;
        Verbose(2, "EOD at %ld\n", getIndex + nGot);
        for (int i=0; i<mFrame.size; i++)
            mWindow[i]->setDataSize(dataSize);
    }

    // At the beginning, which may not be iIndex = 0, prime the windower
    if (mLastIndex < 0)
    {
        for (int i=0; i<mFrame.size; i++)
            mWindow[i]->setData(mInput, i, getIndex, ca);

        // Shift in the first few samples
        for (int a=0; a<nGot-1; a++)
        {
            Verbose(4, "Priming %d\n", a);
            for (int i=0; i<mFrame.size; i++)
            {
                mWindow[i]->shift();
            }
        }
        mLastIndex = iIndex;
    }
    else
    {
        // Sequential constraint; could be relaxed if window were reset
        if (++mLastIndex != iIndex)
            throw Exception("Sequencing error %ld != %ld", iIndex, mLastIndex);
    }

    Verbose(4, "Shifting\n");
    for (int i=0; i<mFrame.size; i++)
    {
        mWindow[i]->setData(mInput, i, getIndex, ca);
        mWindow[i]->shift();
        oData[i] = mWindow[i]->getMean() * mCorrection;
    }

    return true;
}

void Tracter::Minima::Reset(bool iPropagate)
{
    Verbose(2, "Reset\n");
    mLastIndex = -1;
    for (int i=0; i<mFrame.size; i++)
        mWindow[i]->reset();
    CachedComponent<float>::Reset(iPropagate);
}
