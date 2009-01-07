/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "MLPVAD.h"

Tracter::MLPVAD::MLPVAD(Plugin<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(iInput);

    mShowGuts = GetEnv("ShowGuts", 0);

    // Threshold
    mSilThresh = GetEnv("Threshold", 0.5f);
    mSilIndex  = GetEnv("SilenceIndex", 0);

    assert(mSilThresh >= 0.0f && mSilThresh <= 1.0f);
    assert(mSilIndex >= 0 && mSilIndex <=  iInput->GetArraySize());

    // The state machine
    float confirmSpeechTime = GetEnv("ConfirmSpeechTime", 0.02f);
    float confirmSilenceTime = GetEnv("ConfirmSilenceTime", 0.02f);
    mConfirmSpeechTime = SecondsToSamples(confirmSpeechTime);
    mConfirmSilenceTime = SecondsToSamples(confirmSilenceTime);
    mLookAhead = mConfirmSilenceTime;

    int max = std::max(mConfirmSpeechTime, mConfirmSilenceTime);
    MinSize(mInput, mLookAhead+1,  mLookAhead+max);
    mIndex = -1;

    Verbose(1, "%s: LookAhead=%d ConfirmSpeech=%d ConfirmSilence=%d\n",
            mObjectName, mLookAhead,
            mConfirmSpeechTime, mConfirmSilenceTime);
}

void Tracter::MLPVAD::Reset(bool iPropagate)
{
    Verbose(2, "Reset\n");
    mIndex = -1;
    VADStateMachine::Reset(iPropagate);
}

bool Tracter::MLPVAD::UnaryFetch(IndexType iIndex, int iOffset)
{
    Verbose(3, "iIndex %ld\n", iIndex);
    //printf("%i %i\n",iIndex, mIndex);
    assert(iIndex == mIndex+1);
    mIndex = iIndex;
    CacheArea inputArea;

    /* Current value */
    int ret_val = mInput->Read(inputArea, iIndex);
//    if (inputArea.forceDecode)
//    {
//    	ConfirmSilence();
//    	return true;
//    }
    if (ret_val == 0) return false;
    float sil_prob = mInput->GetPointer(inputArea.offset)[mSilIndex];

    /* Update the state machine */
    Update(sil_prob <  mSilThresh);
    VADState* output = GetPointer(iOffset);
    *output = mState;

    if (mShowGuts)
        printf("%ld %e\n",
               iIndex, sil_prob);
    return true;
}
