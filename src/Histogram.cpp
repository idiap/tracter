/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "math.h"
#include "float.h"

#include "Histogram.h"

/**
 * Calculate histogram.
 *
 * Passes data from input to output unchanged, but stores a histogram.
 */
Tracter::Histogram::Histogram(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = iInput->GetArraySize();
    assert(mArraySize >= 0);

    PluginObject::MinSize(iInput, 1);

    mMin = GetEnv("Min", 0.0f);
    mMax = GetEnv("Max", 1.0f);
    mNBins = GetEnv("NBins", 10);
    mPDF = GetEnv("PDF", 1);
    mScale = (float)mNBins/(mMax-mMin);
    mPower = GetEnv("Power", 1.0f);
    mUnPower = GetEnv("UnPower", 0);
    mCount = 0;
    mMinCount = GetEnv("MinCount", 1);
    mMode = GetEnv("Mode", 0);

    mBin.resize(mArraySize);
    for (int i=0; i<mArraySize; i++)
    {
        mBin[i].resize(mNBins);
        for (int j=0; j<mNBins; j++)
            mBin[i][j] = 0.0f;
    }
}

bool Tracter::Histogram::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    CacheArea inputArea;
    if (!mInput->Read(inputArea, iIndex))
        return false;

    // Copy input to output with limits check
    float* input  = mInput->GetPointer(inputArea.offset);
    float* output = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
    {
        // Update histogram. Round down to closest bin
        int bin = mPower == 0.0f
            ? (int)((logf(input[i]) - mMin) * mScale)
            : (int)((powf(input[i], mPower) - mMin) * mScale);
        if ((bin >= 0) && (bin < mNBins))
            mBin[i][bin] += 1.0f;

        // Copy to output
        output[i] = input[i];
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
    for (int j=0; j<mArraySize; j++)
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
        for (int j=0; j<mArraySize; j++)
            printf(" %e", mBin[j][i] >= mMinCount ? mBin[j][i] * scale : 0.0);
        printf("\n");
    }
}
