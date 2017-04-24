/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VITERBIVADGATE_H
#define VITERBIVADGATE_H

#include <algorithm>

#include "CachedComponent.h"
#include "ViterbiVAD.h"

namespace Tracter
{

    /**
     * VAD Gate using Viterbi algorithm to calculate class posterior
     * probabilities smoothed by segment minimum duration and insertion penalty
     * constraints.  Assumes that the inputs are frame-level class posterior
     * probabilties.  Allows frames through from input to output depending on a
     * VAD input.
     */
    class ViterbiVADGate : public CachedComponent<float>
    {
    public:
        ViterbiVADGate(Component<float>* iInput, ViterbiVAD* iVADinput,
                const char* iObjectName = "ViterbiVADGate");

        /**
         * Intercept TimeStamp
         *
         * We need to adjust for the segmenting effect
         */
        TimeType TimeStamp(IndexType iIndex) const
        {
            return ComponentBase::TimeStamp(
                iIndex + std::max<IndexType>(mSpeechTriggered, (IndexType)0)
            );
        }

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);
        virtual void Reset(bool iPropagate);

    private:
        Component<float>* mInput;
        ViterbiVAD* mVADInput;

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
	int mCollar;

        bool gate(IndexType& iIndex);
        bool readVADState(IndexType iIndex);
        bool confirmSpeech(IndexType iIndex);
        bool reconfirmSpeech(IndexType iIndex);
    };
}

#endif /* VITERBIVADGATE_H */
