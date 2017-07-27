/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VADGATE_H
#define VADGATE_H

#include <algorithm>

#include "CachedComponent.h"
#include "VADStateMachine.h"

namespace Tracter
{
    /**
     * VAD Gate.  Allows frames through from input to output depending
     * on a VAD input.
     */
    class VADGate : public CachedComponent<float>
    {
    public:
        VADGate(Component<float>* iInput, VADStateMachine* iVADInput,
                const char* iObjectName = "VADGate");

        /**
         * Intercept TimeStamp
         *
         * We need to adjust for the segmenting effect.  Adding the
         * trigger time will cause ComponentBase::timeStamp() to add
         * on an offset.
         */
        TimeType timeStamp(IndexType iIndex) const
        {
            verbose(2, "TimeStamp: index %ld  trig %ld\n",
                    iIndex, mSpeechTriggered);
            return ComponentBase::timeStamp(
                iIndex + std::max<IndexType>(mSpeechTriggered, (IndexType)0)
            );
        }

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);
        virtual void reset(bool iPropagate);

    private:
        Component<float>* mInput;
        Component<VADState>* mVADInput;

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
