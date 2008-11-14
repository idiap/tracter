/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VADGATE_H
#define VADGATE_H

#include "CachedPlugin.h"
#include "VADStateMachine.h"

namespace Tracter
{
    /**
     * VAD Gate.  Allows frames through from input to output depending
     * on a VAD input.
     */
    class VADGate : public CachedPlugin<float>
    {
    public:
        VADGate(Plugin<float>* iInput, VADStateMachine* iVADInput,
                const char* iObjectName = "VADGate");

    protected:
        PluginObject* GetInput(int iInput);
        bool UnaryFetch(IndexType iIndex, int iOffset);
        virtual void Reset(bool iPropagate);

    private:
        Plugin<float>* mInput;
        Plugin<VADState>* mVADInput;

        bool mEnabled;
        bool mSegmenting;
        bool mRemoveSilence;
        bool mUpstreamEndOfData;

        VADState mState;
        IndexType mSpeechTriggered;
        IndexType mSpeechConfirmed;
        IndexType mSilenceConfirmed;
        IndexType mIndexZero;
        IndexType mSpeechRemoved;

        bool gate(IndexType& iIndex);
        bool readVADState(IndexType iIndex);
        bool confirmSpeech(IndexType iIndex);
        bool reconfirmSpeech(IndexType iIndex);
    };
}

#endif /* VADGATE_H */
