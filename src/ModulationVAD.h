/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MODULATIONVAD_H
#define MODULATIONVAD_H

#include "UnaryPlugin.h"
#include "SlidingDFT.h"
#include "VADStateMachine.h"

namespace Tracter
{
    /**
     * A VAD feature based on energy modulation.  The energy is filtered
     * using the first non-DC bin of a DFT centered at 4Hz because speech
     * is modulated at about 4Hz.  The threshold is based on the noise
     * part of the unfiltered energy.
     */
    class ModulationVAD : public VADStateMachine
    {
    public:
        ModulationVAD(Plugin<float>* iInput,
                      const char* iObjectName = "ModulationVAD");

    protected:

        PluginObject* GetInput(int iInput)
        {
            assert(iInput == 0);
            assert(this->mNInputs == 1);
            return mInput;
        }

        bool UnaryFetch(IndexType iIndex, int iOffset);
        virtual void Reset(bool iPropagate);

    private:
        Plugin<float>* mInput;
        IndexType mIndex;
        int mNBins;
        int mLookAhead;
        int mLookBack;
        SlidingDFT mDFT;
        float mNoise;
        float mNoisePole;
        float mNoiseElop;
        float mThreshold;
        bool mShowGuts;
    };

}

#endif /* MODULATIONVAD_H */
