/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include <cfloat>

#include "Cepstrum.h"

Tracter::Cepstrum::Cepstrum(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    Connect(iInput);

    mNLogData = mInput->Frame().size;

    mFloor = config("Floor", 1e-8f);
    mLogFloor = logf(mFloor);
    mFloored = 0;
    mC0 = config("C0", 1);
    mNCepstra = config("NCepstra", 12);
    mFrame.size = mC0 ? mNCepstra+1 : mNCepstra;

    assert(mNCepstra > 0);
    assert(mNCepstra < mNLogData);

    mLogData = 0;
    mCepstra = 0;
    mFourier.Init(mNLogData, &mLogData, &mCepstra);
}

Tracter::Cepstrum::~Cepstrum() throw()
{
    if (mFloored > 0)
        Verbose(1, "floored %d values < %e\n", mFloored, mFloor);
}

bool Tracter::Cepstrum::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame
    const float* p = mInput->UnaryRead(iIndex);
    if (!p)
        return false;

    // Copy the frame though a log function
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
    for (int i=0; i<mNCepstra; i++)
        oData[i] = mCepstra[i+1];
    if (mC0)
        oData[mNCepstra] = mCepstra[0];

    return true;
}
