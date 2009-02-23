/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VADGATE_H
#define VADGATE_H

#include <algorithm>

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

        /**
         * Intercept TimeStamp
         *
         * We need to adjust for the segmenting effect
         */
        TimeType TimeStamp(IndexType iIndex)
        {
            return PluginObject::TimeStamp(
                iIndex + std::max<IndexType>(mSpeechTriggered, (IndexType)0)
            );
        }

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
        IndexType mSpeechTriggered;  ///< Last speech trigger frame
        IndexType mSpeechConfirmed;  ///< Last speech confirm frame
        IndexType mSilenceConfirmed; ///< Last silence confirm frame
        IndexType mIndexZero;        ///< Zero'th frame from upstream POV
        IndexType mSpeechRemoved;    ///< Number of silence frames removed

        bool gate(IndexType& iIndex);
        bool readVADState(IndexType iIndex);
        bool confirmSpeech(IndexType iIndex);
        bool reconfirmSpeech(IndexType iIndex);
    };
}

#endif /* VADGATE_H */
