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
    objectName(iObjectName);
    mInput = iInput;
    connect(iInput);
    minSize(mInput, 1);

    mNoiseInput = iNoiseInput;
    connect(mNoiseInput);
    minSize(mNoiseInput, 1);

    mShowGuts = config("ShowGuts", 0);

    // Noise tracker
    float noiseTimeConstant = config("NoiseTimeConstant", 0.5f);
    float noiseTime = secondsToFrames(noiseTimeConstant);
    mNoisePole = (noiseTime - 1.0f) / noiseTime;
    mNoiseElop = 1.0f - mNoisePole;

    // Threshold
    float dBThres = config("Threshold", 0.0f);
    mThreshold = dBThres / 10; // decibels to bels

    // The state machine
    float confirmSpeechTime = config("ConfirmSpeechTime", 0.3f);
    float confirmSilenceTime = config("ConfirmSilenceTime", 0.3f);
    mConfirmSpeechTime = secondsToFrames(confirmSpeechTime);
    mConfirmSilenceTime = secondsToFrames(confirmSilenceTime);

    mIndex = -1;

    verbose(2, "ConfirmSpeech=%d ConfirmSilence=%d\n",
            mConfirmSpeechTime, mConfirmSilenceTime);
}

void Tracter::NoiseVAD::reset(bool iPropagate)
{
    verbose(2, "Reset\n");
    mIndex = -1;
    VADStateMachine::reset(iPropagate);
}

bool Tracter::NoiseVAD::unaryFetch(IndexType iIndex, VADState* oData)
{
    verbose(3, "iIndex %lld\n", iIndex);
    if (iIndex != mIndex+1)
        throw Exception(
            "NoiseVAD::UnaryFetch: Index %lld requested; %lld expected\n",
            iIndex, mIndex+1
        );
    mIndex = iIndex;

    /* Prime the noise smoother */
    if (iIndex == 0)
    {
        const float* p = mNoiseInput->unaryRead(iIndex);
        if (!p)
            return false;
        mNoise = log10f(*p);
    }

    /* Update the state machine */
    const float* input = mInput->unaryRead(iIndex);
    if (!input)
        return false;
    update(log10f(*input) > mNoise+mThreshold);
    *oData = mState;

    /* Given the (new) state, update the noise estimate */
    const float* noise = mNoiseInput->unaryRead(iIndex);
    if (!noise)
        return false;
    if (mState == SILENCE_CONFIRMED)
        mNoise = mNoisePole * mNoise + mNoiseElop * log10f(*noise);

    /* Feedback */
    if (mShowGuts)
        printf("%lld %e %e %e %e\n",
               iIndex, log10f(*noise), log10f(*input),
               mNoise, mNoise+mThreshold);
    return true;
}
