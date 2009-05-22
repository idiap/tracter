/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "ModulationVAD.h"

Tracter::ModulationVAD::ModulationVAD(
    Plugin<float>* iInput, const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(iInput);

    /* For a 100Hz frame rate and bin 1 = 4Hz, we have nBins = 100/4 =
     * 25 */
    float freq = GetEnv("Freq", 4.0f);
    float sampleFreq = mSampleFreq / mSamplePeriod;
    mNBins = (int)(sampleFreq / freq + 0.5f);
    mDFT.SetRotation(1, mNBins);
    mLookAhead = mNBins / 2; // Round down
    mLookBack = mNBins - mLookAhead - 1;
    MinSize(mInput, mNBins, mLookAhead);

    mShowGuts = GetEnv("ShowGuts", 0);

    // Noise tracker
    float noiseTimeConstant = GetEnv("NoiseTimeConstant", 0.5f);
    float noiseTime = SecondsToSamples(noiseTimeConstant);
    mNoisePole = (noiseTime - 1.0f) / noiseTime;
    mNoiseElop = 1.0f - mNoisePole;

    // Threshold
    float dBThres = GetEnv("Threshold", 0.0f);
    mThreshold = dBThres / 10; // decibels to bels

    // The state machine
    float confirmSpeechTime = GetEnv("ConfirmSpeechTime", 0.3f);
    float confirmSilenceTime = GetEnv("ConfirmSilenceTime", 0.3f);
    mConfirmSpeechTime = SecondsToSamples(confirmSpeechTime);
    mConfirmSilenceTime = SecondsToSamples(confirmSilenceTime);

    mIndex = -1;

    Verbose(2, "NBins=%d (-%d+%d)"
            " ConfirmSpeech=%d ConfirmSilence=%d\n",
            mNBins, mLookBack, mLookAhead,
            mConfirmSpeechTime, mConfirmSilenceTime);
}

void Tracter::ModulationVAD::Reset(bool iPropagate)
{
    Verbose(2, "Reset\n");
    mIndex = -1;
    VADStateMachine::Reset(iPropagate);
}

bool Tracter::ModulationVAD::UnaryFetch(IndexType iIndex, int iOffset)
{
    Verbose(3, "iIndex %ld\n", iIndex);
    assert(iIndex == mIndex+1);
    mIndex = iIndex;
    CacheArea inputArea;

    float filter = 0.0f;
    float energy = 0.0f;

    if (iIndex == 0)
    {
        /* Reset and prime half the DFT with the first sample */
        mDFT.Reset();
        if (!mInput->Read(inputArea, iIndex))
            return false;
        float* p = mInput->GetPointer(inputArea.offset);
        energy = p[0];
        for (int i=0; i<mLookBack; i++)
            mDFT.Transform(p[0], 0.0f);

        /* Prime the rest of the DFT and the noise estimate with the
         * look-ahead.  The filter gets raw energy, but the noise is
         * tracked in log space to make it less sensitive to
         * spikes. */
        if (!mInput->Read(inputArea, iIndex, mLookAhead+1))
            return false;
        assert(inputArea.len[1] == 0); // The cache should be big enough
        p = mInput->GetPointer(inputArea.offset);
        mNoise = 0.0;
        for (int i=0; i<mLookAhead+1; i++)
        {
            mDFT.Transform(p[i], 0.0f);
            mNoise += log10f(p[i]);
        }
        mNoise /= mLookAhead + 1;
    }

    /* Read the old value - the one just behind the DFT window */
    IndexType oldIndex = iIndex > mLookBack
        ? iIndex - mLookBack - 1
        : 0;
    if (!mInput->Read(inputArea, oldIndex))
        return false;
    float oldVal = *mInput->GetPointer(inputArea.offset);

    /* Current value */
    if (!mInput->Read(inputArea, iIndex))
        return false;
    energy = *mInput->GetPointer(inputArea.offset);

    /* Now the new lookahead value.  Read back from the end until it's
     * found.  It'll be the first hit unless near the end */
    IndexType in = 0;
    for (in=iIndex+mLookAhead; in>=iIndex; in--)
        if (mInput->Read(inputArea, in))
            break;
    float newVal = *mInput->GetPointer(inputArea.offset);

    /* Do the transform */
    complex tmp = mDFT.Transform(newVal, oldVal);
    filter = abs(tmp);
    filter /= mNBins;

    /* Update the state machine */
    Update(log10f(filter) > mNoise+mThreshold);
    VADState* output = GetPointer(iOffset);
    *output = mState;

    /* Given the (new) state, update the noise estimate */
    if (mState == SILENCE_CONFIRMED)
        mNoise = mNoisePole * mNoise + mNoiseElop * log10f(energy);

    if (mShowGuts)
        printf("%ld %e %e %e %e\n",
               iIndex, log10f(energy), log10f(filter),
               mNoise, mNoise+mThreshold);
    return true;
}
