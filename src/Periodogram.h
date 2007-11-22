/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PERIODOGRAM_H
#define PERIODOGRAM_H

#include <vector>

#include <fftw3.h>
#include "UnaryPlugin.h"

/**
 * Uses the FFTW library to calculate a periodogram (aka power
 * spectral density)
 */
class Periodogram : public UnaryPlugin<float, float>
{
public:
    Periodogram(Plugin<float>* iInput,
                const char* iObjectName = "Periodogram");
    ~Periodogram();

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    int mFrameSize;
    int mFramePeriod;
    float* mRealData;
    fftwf_complex* mComplexData;
    fftwf_plan mPlan;
    std::vector<float> mWindow;
};

#endif /* PERIODOGRAM_H */
