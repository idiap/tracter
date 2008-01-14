/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <math.h>
#include "ComplexPeriodogram.h"

ComplexPeriodogram::ComplexPeriodogram(
    Plugin<complex>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, complex>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = GetEnv("FrameSize", 128);
    mFramePeriod = GetEnv("FramePeriod", 40);
    mSamplePeriod *= mFramePeriod;
    assert(mArraySize > 0);
    assert(mFramePeriod > 0);

    PluginObject::MinSize(mInput, mArraySize);

    mInputData =
        (fftwf_complex*)fftwf_malloc(mArraySize * sizeof(fftwf_complex));
    mOutputData =
        (fftwf_complex*)fftwf_malloc(mArraySize * sizeof(fftwf_complex));
    assert(mInputData);
    assert(mOutputData);
    mPlan = fftwf_plan_dft_1d(mArraySize, mInputData, mOutputData,
                              FFTW_FORWARD, 0);

    // Hardwire a Hamming window.  Could be generalised much better.
    // This one is asymmetric.  Should it be symmetric?
    const float PI = 3.14159265358979323846;
    mWindow.resize(mArraySize);
    for (int i=0; i<mArraySize; i++)
        mWindow[i] = 0.54f - 0.46f * cosf(PI * 2.0f * i / (mArraySize - 1));
}

ComplexPeriodogram::~ComplexPeriodogram()
{
    fftwf_destroy_plan(mPlan);
    fftwf_free(mInputData);
    fftwf_free(mOutputData);
}

bool ComplexPeriodogram::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    int readIndex = iIndex * mFramePeriod;
    int got = mInput->Read(inputArea, readIndex, mArraySize);
    if (got < mArraySize)
        return false;

    // Copy the frame into the contiguous array
    complex* p = mInput->GetPointer();
    complex* pp = reinterpret_cast<complex*>(mInputData);
    for (int i=0; i<inputArea.len[0]; i++)
        pp[i] = p[inputArea.offset+i] * mWindow[i];
    for (int i=0; i<inputArea.len[1]; i++)
        pp[i+inputArea.len[0]] = p[i] * mWindow[i+inputArea.len[0]];

    // Do the DFT
    fftwf_execute(mPlan);

    // Compute periodogram
    float* cache = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
        cache[i] =
                mOutputData[i][0] * mOutputData[i][0] +
                mOutputData[i][1] * mOutputData[i][1];

    return true;
}
