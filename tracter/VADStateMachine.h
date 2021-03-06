/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VADSTATEMACHINE_H
#define VADSTATEMACHINE_H

#include <tracter/CachedComponent.h>

namespace Tracter
{
    /**
     * Defines a four-state FSM implementing a kind of Schmitt trigger
     * for VAD.
     */
    enum VADState
    {
        SPEECH_TRIGGERED,
        SPEECH_CONFIRMED,
        SILENCE_TRIGGERED,
        SILENCE_CONFIRMED
    };

    /**
     * State Machine for VAD.  Implements a state machine requiring a
     * minimum number of speech or silence frames before confirming speech
     * or silence.
     */
    class VADStateMachine : public CachedComponent<VADState>
    {
    public:
        VADStateMachine();
        void reset(bool iPropagate);
        void confirmSilence();

        int confirmSpeechTime() { return mConfirmSpeechTime; };
        int confirmSilenceTime() { return mConfirmSilenceTime; };

    protected:
        void update(bool iSpeech);

        VADState mState;
        int mTime;
        int mConfirmSpeechTime;
        int mConfirmSilenceTime;
    };
}

#endif /* VADSTATEMACHINE_H */
