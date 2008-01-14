/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ComplexSample.h"

ComplexSample::ComplexSample(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<complex, float>(iInput)
{
    mObjectName = iObjectName;
    mSampleFreq /= 4;
}

void ComplexSample::MinSize(int iSize, int iReadAhead)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    PluginObject::MinSize(iSize, iReadAhead);

    // We expect the input buffer to be at least the size of each request
    assert(mInput);
    PluginObject::MinSize(mInput, iSize * 4 - 2, 0);
}

int ComplexSample::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input data
    int readIndex = iIndex * 4;
    int readLen = iOutputArea.Length() * 4 - 2;
    int lenGot = mInput->Read(inputArea, readIndex, readLen);

    CacheIterator<float> input(mInput, inputArea);
    CacheIterator<complex> output(this, iOutputArea);

    // Copy the first two of each group of 4 into the complex output
    int count = 0;
    for (int i=0; i+1<lenGot; i+=4)
    {
        float real = *input++;
        float imag = *input++;
        input++;
        input++;
        *output++ = complex(real, imag);
        count++;
    }

    return count;
}
