/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <math.h>
#include <float.h>
#include "Cepstrum.h"

Cepstrum::Cepstrum(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mNLogData = mInput->GetArraySize();

    mFloor = GetEnv("Floor", 1e-8f);
    mLogFloor = logf(mFloor);
    mFloored = 0;
    mC0 = GetEnv("C0", 1);
    mNCepstra = GetEnv("NCepstra", 12);
    mArraySize = mC0 ? mNCepstra+1 : mNCepstra;

    assert(mNCepstra > 0);
    assert(mNCepstra < mNLogData);
    MinSize(mInput, 1);

    mLogData = 0;
    mCepstra = 0;
    mFourier.Init(mNLogData, &mLogData, &mCepstra);
}

Cepstrum::~Cepstrum()
{
    if ((Tracter::sVerbose > 0) && (mFloored > 0))
        printf("Cepstrum: floored %d values < %e\n", mFloored, mFloor);
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
        if (p[i] > mFloor)
            mLogData[i] = logf(p[i]);
        else
        {
            mLogData[i] = mLogFloor;
            mFloored++;
        }

    // Do the DCT
    mFourier.Transform();

    // Copy to output in HTK order (C0 last, if at all)
    float* cache = GetPointer(iOffset);
    for (int i=0; i<mNCepstra; i++)
        cache[i] = mCepstra[i+1];
    if (mC0)
        cache[mNCepstra] = mCepstra[0];

    return true;
}
