/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef CEPSTRUM_H
#define CEPSTRUM_H

#include "Fourier.h"
#include "UnaryPlugin.h"

/**
 * Generate the cepstrum of an input
 */
class Cepstrum : public UnaryPlugin<float, float>
{
public:
    Cepstrum(Plugin<float>* iInput, const char* iObjectName = "Cepstrum");
    virtual ~Cepstrum();

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    float mFloor;
    float mLogFloor;
    int mFloored;
    int mNLogData;
    int mNCepstra;
    float* mLogData;
    float* mCepstra;
    bool mC0;
    Fourier mFourier;
};

#endif /* CEPSTRUM_H */
