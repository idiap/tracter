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
    class ModulationVAD : public UnaryPlugin<VADState, float>,
                          public VADStateMachine
    {
    public:
        ModulationVAD(Plugin<float>* iInput,
                      const char* iObjectName = "ModulationVAD");

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
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
