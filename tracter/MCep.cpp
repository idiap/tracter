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
    objectName(iObjectName);
    mInput = iInput;
    connect(mInput);

    // Array sizing
    mC0 = config("C0", 1);
    int nCepstra = config("NCepstra", 12);
    mFrame.size = mC0 ? nCepstra+1 : nCepstra;
    assert(nCepstra > 0);
    mInData.resize(mInput->frame().size);
    mCepstra1.resize(nCepstra+1);
    mCepstra2.resize(nCepstra+1);

    // Parameters, defaults from mcep executable
    mAlpha = config("Alpha", 0.35f);
    mGamma = config("Gamma", 1.0f);
    mMinIter = config("MinIter", 2);
    mMaxIter = config("MaxIter", 30);
    mEndCondition = config("EndCondition", 0.001f);
    mSmall = config("Small", 0.0f);
    mDetMin = config("DetMin", 0.000001f);

    // Check gamma
    if (mGamma > 0.0)
        verbose(1, "Gamma > 0.0; using mcep()\n");
    else
        if (mGamma != 0.0)
            verbose(1, "Gamma < 0.0; using mgcep() and mgc2mgc()\n");
        else
            verbose(1, "Gamma == 0.0; using mgcep() only\n");
}

bool Tracter::MCep::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame - a periodogram
    const float* p = mInput->unaryRead(iIndex);
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
            mEndCondition, 1, mSmall, mDetMin,
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
            mEndCondition, 1, mSmall, mDetMin,
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
