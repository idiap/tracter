/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VITERBIVAD_H
#define VITERBIVAD_H

#include <algorithm>
#include <deque>

#include "CachedComponent.h"
#include "VADStateMachine.h" // we use the state machine states, though only
                             // speech confirmed/silence confirmed are used

namespace Tracter
{

    /**
     * Statistical based voice activity detection.  Inputs are the silence
     * class posterior probabilities e.g. from an MLP.  Voicing decision is
     * smoothed using HMM with sil/speech minimum duration.
     */
    class ViterbiVAD : public CachedComponent<VADState>
    {
    private:
        int mLookAhead;      // how far to lookahead before making a traceback
        int mSilStates;      // silence minimum duration
        int mSpeechStates;   // speech minimum duration
        float mSilPrior;       // silence class prior probability
        float mSpeechPrior;    // speech class prior probability
        float mInsPen;         // segment insertion penalty
 
        //  additional variables that need not be explained
        int mBestState;
        int mEndSpeech;      
        int mEndSil;
        VADState mState;
        IndexType mEndOfData;
        IndexType mIndex;
        IndexType mLookAheadIndex;

        std::vector<float> score; // contains the score of states for viterbi
                                  // search at current frame
        std::vector<float> tmp_score; // temporary container for calculating
                                      // scores for next frame
        std::deque< std::vector<int> > traceback; // container for traceback
                                                  // information

    public:
        ViterbiVAD(Component<float>* iInput,
                   const char* iObjectName = "ViterbiVAD");

    protected:
        bool unaryFetch(IndexType iIndex, VADState* oData);
        virtual void reset(bool iPropagate);

    private:
	Component<float>* mInput;
	bool getVADState(IndexType iIndex);
	void doForward(IndexType iIndex, float pSil = -1.0);
	bool doTraceback(IndexType iIndex, VADState &vad_state);
    };
}

#endif /* VITERBIVAD_H */
