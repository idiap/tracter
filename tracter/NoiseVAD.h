/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef NOISEVAD_H
#define NOISEVAD_H

#include "tracter/VADStateMachine.h"

namespace Tracter
{
    /**
     * A VAD with a noise tracker.
     *
     * The VAD threshold is based on the noise input, smoothed
     * according to the VAD state.
     */
    class NoiseVAD : public VADStateMachine
    {
    public:
        NoiseVAD(Component<float>* iInput, Component<float>* iNoiseInput,
                 const char* iObjectName = "NoiseVAD");

    protected:

        bool unaryFetch(IndexType iIndex, VADState* oData);
        virtual void reset(bool iPropagate);

        void dotHook()
        {
            VADStateMachine::dotHook();
            dotRecord(1, "pole=%.2f", mNoisePole);
            dotRecord(1, "thres=%.2f", mThreshold);
        }

    private:
        Component<float>* mInput;
        Component<float>* mNoiseInput;
        IndexType mIndex;
        float mNoise;
        float mNoisePole;
        float mNoiseElop;
        float mThreshold;
        bool mShowGuts;
    };
}

#endif /* NOISEVAD_H */
