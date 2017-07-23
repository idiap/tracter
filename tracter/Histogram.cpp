/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cmath>
#include <cfloat>

#include "Histogram.h"

/**
 * Calculate histogram.
 *
 * Passes data from input to output unchanged, but stores a histogram.
 */
Tracter::Histogram::Histogram(Component<float>* iInput, const char* iObjectName)
{
    objectName(iObjectName);
    mInput = iInput;
    Connect(mInput);
    mFrame.size = iInput->Frame().size;
    assert(mFrame.size >= 0);

    mMin = config("Min", 0.0f);
    mMax = config("Max", 1.0f);
    mNBins = config("NBins", 10);
    mPDF = config("PDF", 1);
    mScale = (float)mNBins/(mMax-mMin);
    mPower = config("Power", 1.0f);
    mUnPower = config("UnPower", 0);
    mCount = 0;
    mMinCount = config("MinCount", 1);
    mMode = config("Mode", 0);

    mBin.resize(mFrame.size);
    for (int i=0; i<mFrame.size; i++)
    {
        mBin[i].resize(mNBins);
        for (int j=0; j<mNBins; j++)
            mBin[i][j] = 0.0f;
    }
}

bool Tracter::Histogram::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    CacheArea inputArea;
    if (!mInput->Read(inputArea, iIndex))
        return false;

    // Copy input to output with limits check
    float* input  = mInput->GetPointer(inputArea.offset);
    for (int i=0; i<mFrame.size; i++)
    {
        // Update histogram. Round down to closest bin
        int bin = -1;
        if (mPower == 0.0f)
            bin = input[i] > FLT_MIN
                ? (int)((logf(input[i]) - mMin) * mScale)
                : -1;
        else
            bin = (int)((powf(input[i], mPower) - mMin) * mScale);
        if ((bin >= 0) && (bin < mNBins))
            mBin[i][bin] += 1.0f;

        // Copy to output
        oData[i] = input[i];
    }
    mCount++;

    return true;
}

Tracter::Histogram::~Histogram() throw()
{
    if (mMode)
        writeMode();
    else
        writeHistogram();
}

void Tracter::Histogram::writeMode()
{
    for (int j=0; j<mFrame.size; j++)
    {
        int maxBin = -1;
        float maxVal = -1;
        for (int i=0; i<mNBins; i++)
        {
            if (mBin[j][i] > maxVal)
            {
                maxVal = mBin[j][i];
                maxBin = i;
            }
        }
        float binVal = ((float)maxBin + 0.5) / mScale + mMin;
        if (mUnPower)
            binVal = mPower == 0.0f ? expf(binVal) : powf(binVal, 1.0f/mPower);
        printf("%e\n", binVal);
    }
}

void Tracter::Histogram::writeHistogram()
{
    assert(mMax > mMin);

    /* For a PDF, we need to divide by the sample count, but also by
     * the width of one histogram bin */
    float scale = mPDF
        ? mScale / mCount
        : 1.0f;

    for (int i=0; i<mNBins; i++)
    {
        float binVal = ((float)i + 0.5) / mScale + mMin;
        if (mUnPower)
            binVal = mPower == 0.0f ? expf(binVal) : powf(binVal, 1.0f/mPower);
        printf("%e", binVal);
        for (int j=0; j<mFrame.size; j++)
            printf(" %e", mBin[j][i] >= mMinCount ? mBin[j][i] * scale : 0.0);
        printf("\n");
    }
}
