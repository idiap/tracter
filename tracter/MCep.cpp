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
    Component<float>* iInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(mInput);

    // Array sizing
    mC0 = GetEnv("C0", 1);
    int nCepstra = GetEnv("NCepstra", 12);
    mFrame.size = mC0 ? nCepstra+1 : nCepstra;
    assert(nCepstra > 0);
    mInData.resize(mInput->Frame().size);
    mCepstra1.resize(nCepstra+1);
    mCepstra2.resize(nCepstra+1);

    // Parameters, defaults from mcep executable
    mAlpha = GetEnv("Alpha", 0.35f);
    mGamma = GetEnv("Gamma", 1.0f);
    mMinIter = GetEnv("MinIter", 2);
    mMaxIter = GetEnv("MaxIter", 30);
    mEndCondition = GetEnv("EndCondition", 0.001f);
    mSmall = GetEnv("Small", 0.0f);
    mDetMin = GetEnv("DetMin", 0.000001f);

    // Check gamma
    if (mGamma > 0.0)
        Verbose(1, "Gamma > 0.0; using mcep()\n");
    else
        if (mGamma != 0.0)
            Verbose(1, "Gamma < 0.0; using mgcep() and mgc2mgc()\n");
        else
            Verbose(1, "Gamma == 0.0; using mgcep() only\n");
}

bool Tracter::MCep::UnaryFetch(IndexType iIndex, float* oData)
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

    // Run SPTK's mcep or mgcep
    int l = (mInData.size() - 1) * 2; // 129 -> 256
    int m = mCepstra1.size() - 1;     // 13d = 12th order + C0
    double* b = &mCepstra1[0];
    double* c = &mCepstra2[0];
    if (mGamma > 0.0)
    {
        mcep(
            f, l,
            c, m,
            mAlpha,
            mMinIter, mMaxIter,
            mEndCondition, mSmall, mDetMin,
            4 // = Periodogram
        );
    }
    else
    {
        mgcep(
            f, l,
            b, m,
            mAlpha, mGamma, l - 1,
            mMinIter, mMaxIter,
            mEndCondition, mSmall, mDetMin,
            4 // = Periodogram
        );
        ignorm(b, b, m, mGamma);      // K, b'r --> br
        if (mAlpha != 0.0)
            b2mc(b, b, m, mAlpha);    // br --> c~r

        if (mGamma != 0.0)
            mgc2mgc(
                b, m, mAlpha, mGamma, // Old values
                c, m, mAlpha, 0.0     // New values
            );
        else
            c = b;
    }

    // Copy to output in HTK order (C0 last, if at all)
    for (int i=0; i<m; i++)
        oData[i] = c[i+1];
    if (mC0)
        oData[m] = c[0];

    return true;
}
