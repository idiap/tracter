/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "MAPSpectrum.h"

PluginObject* MAPSpectrum::GetInput(int iInput)
{
    // Enumerate the inputs
    switch (iInput)
    {
        case 0:
            return mInput1;
        case 1:
            return mInput2;
        default:
            assert(0);
    }

    // Should never get here
    return 0;
}

MAPSpectrum::MAPSpectrum(
    Plugin<float>* iInput1,
    Plugin<float>* iInput2,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mArraySize = iInput1->GetArraySize();

    Connect(iInput1);
    Connect(iInput2);

    mInput1 = iInput1;
    mInput2 = iInput2;

    mGamma = GetEnv("Gamma", 0);
    mAverage = GetEnv("Average", 0);
    float alpha = GetEnv("Alpha", 1.1f);
    float snr = GetEnv("SNR", 1.0f);
    mMAPNoise.SetAlpha(alpha);
    mMAPNoise.SetSNR(snr);

    for (int i=0; i<mNInputs; i++)
        MinSize(GetInput(i), 1);
}

bool MAPSpectrum::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);
    CacheArea inputArea;
    float* cache = GetPointer(iOffset);

    // Start with the second input
    if (mInput2->Read(inputArea, iIndex) == 0)
        return false;
    float *p2 = mInput2->GetPointer(inputArea.offset);

    // Now the first input
    if (mInput1->Read(inputArea, iIndex) == 0)
        return false;
    float *p1 = mInput1->GetPointer(inputArea.offset);

    // Find the average noise
    float av = 0.0f;
    if (mAverage)
    {
        for (int i=0; i<mArraySize; i++)
            av += p2[i];
        av /= mArraySize;
    }

    // Do the estimation
    for (int i=0; i<mArraySize; i++)
    {
        float speech;
        if (mAverage)
            speech = mGamma
                ? mMAPNoise.GammaPrior(p1[i], p2[i], av)
                : mMAPNoise.InverseGammaPrior(p1[i], p2[i], av);
        else
            speech = mGamma
                ? mMAPNoise.MagGammaPrior(p1[i], p2[i])
                //? mMAPNoise.GammaPrior(p1[i], p2[i])
                : mMAPNoise.InverseGammaPrior(p1[i], p2[i]);
        cache[i] = speech; // / p2[i];  // SNR!
    }

    return true;
}
