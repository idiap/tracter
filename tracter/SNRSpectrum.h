/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SNRSPECTRUM_H
#define SNRSPECTRUM_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * SNR spectral estimator
     */
    class SNRSpectrum : public CachedComponent<float>
    {
    public:
        SNRSpectrum(
            Component<float>* iPowerInput, Component<float>* iNoiseInput,
            const char* iObjectName = "SNRSpectrum"
        );
        virtual ~SNRSpectrum() throw() {}

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mPowerInput;
        Component<float>* mNoiseInput;
    };
}

#endif /* SNRSPECTRUM_H */
