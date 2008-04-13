/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PLP_H
#define PLP_H

#include "Fourier.h"
#include "UnaryPlugin.h"

/**
 * PLP analysis from a warped spectrum
 */
class PLP : public UnaryPlugin<float, float>
{
public:
    PLP(Plugin<float>* iInput, const char* iObjectName = "PLP");

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    int mOrder;
    int mNCompressed;
    int mNCepstra;
    bool mC0;
    float mCompressionPower;
    float* mCompressed;
    float* mAutoCorrelation;
    Fourier mFourier;
};

#endif /* PLP_H */
