/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef COMPLEXPERIODOGRAM_H
#define COMPLEXPERIODOGRAM_H

#include <vector>

#include "Fourier.h"
#include "CachedComponent.h"
#include "ComplexSample.h" // for the complex definitions

namespace Tracter
{
    /**
     * Calculate a periodogram (aka power spectral density).  The inputs
     * are complex numbers from, e.g., quadrature sampling.
     */
    class ComplexPeriodogram : public CachedComponent<float>
    {
    public:
        ComplexPeriodogram(Component<complex>* iInput,
                           const char* iObjectName = "ComplexPeriodogram");
        virtual ~ComplexPeriodogram() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<complex>* mInput;
        complex* mInputData;
        complex* mOutputData;
        Fourier mFourier;
        std::vector<float> mWindow;
    };
}

#endif /* COMPLEXPERIODOGRAM_H */
