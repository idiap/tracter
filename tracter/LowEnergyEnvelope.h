/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef LOWENERGYENVELOPE_H
#define LOWENERGYENVELOPE_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Use the Low Energy Envelope method to estimate noise
     */
    class LowEnergyEnvelope : public CachedComponent<float>
    {
    public:
        LowEnergyEnvelope(
            Component<float>* iInput,
            const char* iObjectName = "LowEnergyEnvelope"
        );
        virtual ~LowEnergyEnvelope() throw() {}

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        int mNWindow;
        int mNGamma;
        float mCorrection;
        std::vector< std::vector<float> > mTmp;
    };
}

#endif /* LOWENERGYENVELOPE_H */
