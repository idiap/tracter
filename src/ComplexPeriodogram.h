/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef COMPLEXPERIODOGRAM_H
#define COMPLEXPERIODOGRAM_H

#include <vector>

#include <fftw3.h>
#include "UnaryPlugin.h"
#include "ComplexSample.h" // for the complex definitions

/**
 * Uses the FFTW library to calculate a periodogram (aka power
 * spectral density).  The inputs are complex numbers from, e.g.,
 * quadrature sampling.
 */
class ComplexPeriodogram : public UnaryPlugin<float, complex>
{
public:
    ComplexPeriodogram(Plugin<complex>* iInput,
                       const char* iObjectName = "ComplexPeriodogram");
    ~ComplexPeriodogram();

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    int mFrameSize;
    int mFramePeriod;
    fftwf_complex* mInputData;
    fftwf_complex* mOutputData;
    fftwf_plan mPlan;
    std::vector<float> mWindow;
};

#endif /* COMPLEXPERIODOGRAM_H */
