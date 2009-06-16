/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef NOISEVAD_H
#define NOISEVAD_H

#include "VADStateMachine.h"

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
        NoiseVAD(Plugin<float>* iInput, Plugin<float>* iNoiseInput,
                 const char* iObjectName = "NoiseVAD");

    protected:

        PluginObject* GetInput(int iInput)
        {
            switch (iInput)
            {
            case 0:
                assert(mInput);
                return mInput;
            case 1:
                assert(this->mNInputs == 2);
                assert(mNoiseInput);
                return mNoiseInput;
            default:
                assert(0);
            }
        }

        bool UnaryFetch(IndexType iIndex, int iOffset);
        virtual void Reset(bool iPropagate);

        void DotHook()
        {
            VADStateMachine::DotHook();
            DotRecord(1, "pole=%.2f", mNoisePole);
            DotRecord(1, "thres=%.2f", mThreshold);
        }

    private:
        Plugin<float>* mInput;
        Plugin<float>* mNoiseInput;
        IndexType mIndex;
        float mNoise;
        float mNoisePole;
        float mNoiseElop;
        float mThreshold;
        bool mShowGuts;
    };
}

#endif /* NOISEVAD_H */
