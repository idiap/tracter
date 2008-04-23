/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef LOWENERGYENVELOPE_H
#define LOWENERGYENVELOPE_H

#include "UnaryPlugin.h"

/**
 * Use the Low Energy Envelope method to estimate noise
 */
class LowEnergyEnvelope : public UnaryPlugin<float, float>
{
public:
    LowEnergyEnvelope(
        Plugin<float>* iInput, const char* iObjectName = "LowEnergyEnvelope"
    );

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    int mNWindow;
    int mNGamma;
    float mCorrection;
    std::vector< std::vector<float> > mTmp;
};

#endif /* LOWENERGYENVELOPE_H */
