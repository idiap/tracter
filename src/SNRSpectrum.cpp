/*
 * Copyright 2009 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cfloat>

#include "SNRSpectrum.h"

Tracter::SNRSpectrum::SNRSpectrum(
    Component<float>* iPowerInput,
    Component<float>* iNoiseInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mFrame.size = iPowerInput->Frame().size;

    mPowerInput = iPowerInput;
    mNoiseInput = iNoiseInput;

    Connect(iPowerInput, 1);
    Connect(iNoiseInput, 1);
}

bool Tracter::SNRSpectrum::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Start with the noise input
    const float* ni = mNoiseInput->UnaryRead(iIndex);
    if (!ni)
        return false;

    // Now the power input
    const float* pi = mPowerInput->UnaryRead(iIndex);
    if (!pi)
        return false;

    // Calculate SNR.  Make sure the noise is floored to non-zero.
    for (int i=0; i<mFrame.size; i++)
    {
        float n  = std::max(FLT_MIN, ni[i]);
        oData[i] = std::max(pi[i] / n, 1.0f);
    }

    return true;
}
