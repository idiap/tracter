/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cassert>

#include "VADStateMachine.h"

Tracter::VADStateMachine::VADStateMachine()
{
    mState = SILENCE_CONFIRMED;
    mTime = 0;
    mConfirmSpeechTime = 0;
    mConfirmSilenceTime = 0;
}

void Tracter::VADStateMachine::Reset()
{
    mState = SILENCE_CONFIRMED;
    //printf("RESET -> SIL_CONF\n");
    mTime = 0;
}

void Tracter::VADStateMachine::Update(bool iSpeech)
{
    switch (mState)
    {
    case SILENCE_TRIGGERED:
        if (!iSpeech)
        {
            if (mTime++ >= mConfirmSilenceTime)
            {
                mState = SILENCE_CONFIRMED;
                //printf("SIL_TRIG -> SIL_CONF\n");
            }
        }
        else
        {
            mState = SPEECH_CONFIRMED;
            //printf("SIL_TRIG -> SP_CONF\n");
        }
        break;

    case SILENCE_CONFIRMED:
        if (iSpeech)
        {
            mState = SPEECH_TRIGGERED;
            //printf("SIL_CONF -> SP_TRIG\n");
            mTime = 1;
        }
        break;

    case SPEECH_TRIGGERED:
        if (iSpeech)
        {
            if (mTime++ >= mConfirmSpeechTime)
            {
                mState = SPEECH_CONFIRMED;
                //printf("SP_TRIG -> SP_CONF\n");
            }
        }
        else
        {
            mState = SILENCE_CONFIRMED;
            //printf("SP_TRIG -> SIL_CONF\n");
        }
        break;

    case SPEECH_CONFIRMED:
        if (!iSpeech)
        {
            mState = SILENCE_TRIGGERED;
            //printf("SP_CONF -> SIL_TRIG\n");
            mTime = 1;
        }
        break;

    default:
        assert(0);
    }

    //printf("Machine: mState is %d\n", mState);
}
