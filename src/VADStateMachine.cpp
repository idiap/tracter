/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>
#include "VADStateMachine.h"

VADStateMachine::VADStateMachine()
{
    mState = SILENCE_CONFIRMED;
    mTime = 0;
    mConfirmSpeechTime = 0;
    mConfirmSilenceTime = 0;
}

void VADStateMachine::Update(bool iSpeech)
{
    switch (mState)
    {
    case SILENCE_TRIGGERED:
        if (!iSpeech)
        {
            if (mTime++ >= mConfirmSilenceTime)
                mState = SILENCE_CONFIRMED;
        }
        else
            mState = SPEECH_CONFIRMED;
        break;

    case SILENCE_CONFIRMED:
        if (iSpeech)
        {
            mState = SPEECH_TRIGGERED;
            mTime = 1;
        }
        break;

    case SPEECH_TRIGGERED:
        if (iSpeech)
        {
            if (mTime++ >= mConfirmSpeechTime)
                mState = SPEECH_CONFIRMED;
        }
        else
            mState = SILENCE_CONFIRMED;
        break;

    case SPEECH_CONFIRMED:
        if (!iSpeech)
        {
            mState = SILENCE_TRIGGERED;
            mTime = 1;
        }
        break;

    default:
        assert(0);
    }
}
