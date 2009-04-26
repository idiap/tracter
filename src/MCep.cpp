/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio> // for SPTK
extern "C" {
#include <SPTK.h>
}
#include "MCep.h"

Tracter::MCep::MCep(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;

    // Array sizing
    mC0 = GetEnv("C0", 1);
    int nCepstra = GetEnv("NCepstra", 12);
    mArraySize = mC0 ? nCepstra+1 : nCepstra;
    assert(nCepstra > 0);
    MinSize(mInput, 1);
    mInData.resize(mInput->GetArraySize());
    mCepstra.resize(nCepstra+1);

    // Parameters, defaults from mcep executable
    mAlpha = GetEnv("Alpha", 0.35f);
    mMinIter = GetEnv("MinIter", 2);
    mMaxIter = GetEnv("MaxIter", 30);
    mEndCondition = GetEnv("EndCondition", 0.001f);
    mSmall = GetEnv("Small", 0.0f);
    mDetMin = GetEnv("DetMin", 0.000001f);
}

bool Tracter::MCep::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    // Read the input frame - a periodogram
    const float* p = mInput->UnaryRead(iIndex);
    if (!p)
        return false;

    // Copy to store (float -> double)
    double* f = &mInData[0];
    for (size_t i=0; i<mInData.size(); i++)
        f[i] = p[i];

    // Run SPTK's mcep
    mcep(
        f, (mInData.size() - 1) * 2,     // 129 -> 256
        &mCepstra[0], mCepstra.size()-1, // 13d = 12 + C0
        mAlpha, mMinIter, mMaxIter,
        mEndCondition, mSmall, mDetMin,
        4                                // = Periodogram
    );

    // Copy to output in HTK order (C0 last, if at all)
    float* cache = GetPointer(iOffset);
    for (size_t i=0; i<mCepstra.size()-1; i++)
        cache[i] = mCepstra[i+1];
    if (mC0)
        cache[mCepstra.size()-1] = mCepstra[0];

    return true;
}
