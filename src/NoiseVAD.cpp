/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cmath>

#include "NoiseVAD.h"

Tracter::NoiseVAD::NoiseVAD(
    Component<float>* iInput, Component<float>* iNoiseInput, const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(iInput);
    MinSize(mInput, 1);

    mNoiseInput = iNoiseInput;
    Connect(mNoiseInput);
    MinSize(mNoiseInput, 1);

    mShowGuts = GetEnv("ShowGuts", 0);

    // Noise tracker
    float noiseTimeConstant = GetEnv("NoiseTimeConstant", 0.5f);
    float noiseTime = SecondsToFrames(noiseTimeConstant);
    mNoisePole = (noiseTime - 1.0f) / noiseTime;
    mNoiseElop = 1.0f - mNoisePole;

    // Threshold
    float dBThres = GetEnv("Threshold", 0.0f);
    mThreshold = dBThres / 10; // decibels to bels

    // The state machine
    float confirmSpeechTime = GetEnv("ConfirmSpeechTime", 0.3f);
    float confirmSilenceTime = GetEnv("ConfirmSilenceTime", 0.3f);
    mConfirmSpeechTime = SecondsToFrames(confirmSpeechTime);
    mConfirmSilenceTime = SecondsToFrames(confirmSilenceTime);

    mIndex = -1;

    Verbose(2, "ConfirmSpeech=%d ConfirmSilence=%d\n",
            mConfirmSpeechTime, mConfirmSilenceTime);
}

void Tracter::NoiseVAD::Reset(bool iPropagate)
{
    Verbose(2, "Reset\n");
    mIndex = -1;
    VADStateMachine::Reset(iPropagate);
}

bool Tracter::NoiseVAD::UnaryFetch(IndexType iIndex, VADState* oData)
{
    Verbose(3, "iIndex %ld\n", iIndex);
    assert(iIndex == mIndex+1);
    mIndex = iIndex;

    /* Prime the noise smoother */
    if (iIndex == 0)
    {
        const float* p = mNoiseInput->UnaryRead(iIndex);
        if (!p)
            return false;
        mNoise = log10f(*p);
    }

    /* Update the state machine */
    const float* input = mInput->UnaryRead(iIndex);
    if (!input)
        return false;
    Update(log10f(*input) > mNoise+mThreshold);
    *oData = mState;

    /* Given the (new) state, update the noise estimate */
    const float* noise = mNoiseInput->UnaryRead(iIndex);
    if (!noise)
        return false;
    if (mState == SILENCE_CONFIRMED)
        mNoise = mNoisePole * mNoise + mNoiseElop * log10f(*noise);

    /* Feedback */
    if (mShowGuts)
        printf("%ld %e %e %e %e\n",
               iIndex, log10f(*noise), log10f(*input),
               mNoise, mNoise+mThreshold);
    return true;
}
