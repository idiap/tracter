/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "math.h"
#include "float.h"

#include "Histogram.h"

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
    mCount = 0;
    mMinCount = GetEnv("MinCount", 1);

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
        int bin = (int)((powf(input[i], mPower) - mMin) * mScale);
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
    write();
}

void Tracter::Histogram::write()
{
    assert(mMax > mMin);

    /* For a PDF, we need to divide by the sample count, but also by
     * the width of one histogram bin */
    float scale = mPDF
        ? mScale / mCount
        : 1.0f;

    for (int i=0; i<mNBins; i++)
    {
        printf("%e", ((float)i + 0.5) / mScale + mMin);
        for (int j=0; j<mArraySize; j++)
            printf(" %e", mBin[j][i] >= mMinCount ? mBin[j][i] * scale : 0.0);
        printf("\n");
    }
}
