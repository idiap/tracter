/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "MLPVAD.h"

Tracter::MLPVAD::MLPVAD(Component<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mInput = iInput;

    mShowGuts = GetEnv("ShowGuts", 0);

    // Threshold
    mThreshold = GetEnv("Threshold", 0.5f);
    mInputIndex = GetEnv("Index", 0);
    mSpeech = GetEnv("Speech", 0);

    assert(mThreshold >= 0.0f && mThreshold <= 1.0f);
    assert(mInputIndex >= 0 && mInputIndex < iInput->Frame().size);

    // The state machine
    float confirmSpeechTime = GetEnv("ConfirmSpeechTime", 0.02f);
    float confirmSilenceTime = GetEnv("ConfirmSilenceTime", 0.02f);
    mConfirmSpeechTime = SecondsToFrames(confirmSpeechTime);
    mConfirmSilenceTime = SecondsToFrames(confirmSilenceTime);
    mLookAhead = mConfirmSilenceTime;

    int max = std::max(mConfirmSpeechTime, mConfirmSilenceTime);
    Connect(mInput, mLookAhead+1,  mLookAhead+max);
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

bool Tracter::MLPVAD::UnaryFetch(IndexType iIndex, VADState* oData)
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
    float prob = mInput->GetPointer(inputArea.offset)[mInputIndex];

    /* Update the state machine */
    Update( mSpeech
            ? prob >  mThreshold
            : prob <  mThreshold );
    *oData = mState;

    if (mShowGuts)
        printf("%ld %e\n",
               iIndex, prob);
    return true;
}
