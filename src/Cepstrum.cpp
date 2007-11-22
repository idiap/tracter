/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <math.h>
#include "Cepstrum.h"

Cepstrum::Cepstrum(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mNLogData = mInput->GetArraySize();

    mC0 = GetEnv("C0", 1);
    mNCepstra = GetEnv("NCepstra", 12);
    mArraySize = mC0 ? mNCepstra+1 : mNCepstra;

    assert(mNCepstra > 0);
    assert(mNCepstra < mNLogData);
    MinSize(mInput, 1);

    mLogData = (float*)fftwf_malloc(mNLogData * sizeof(float));
    mCepstra = (float*)fftwf_malloc(mNLogData * sizeof(float));
    assert(mLogData);
    assert(mCepstra);
    mPlan = fftwf_plan_r2r_1d(mNLogData, mLogData, mCepstra, FFTW_REDFT10, 0);
}

Cepstrum::~Cepstrum()
{
    fftwf_destroy_plan(mPlan);
    fftwf_free(mLogData);
    fftwf_free(mCepstra);
}

bool Cepstrum::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    int one = mInput->Read(inputArea, iIndex);
    if (one < 1)
        return false;

    // Copy the frame though a log function
    float* p = mInput->GetPointer(inputArea.offset);
    for (int i=0; i<mNLogData; i++)
        mLogData[i] = logf(p[i]);

    // Do the DCT
    fftwf_execute(mPlan);

    // Copy to output in HTK order (C0 last, if at all)
    float* cache = GetPointer(iOffset);
    for (int i=0; i<mNCepstra; i++)
        cache[i] = mCepstra[i+1];
    if (mC0)
        cache[mNCepstra] = mCepstra[0];

    return true;
}
