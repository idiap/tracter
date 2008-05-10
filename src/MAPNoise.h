/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MAPNOISE_H
#define MAPNOISE_H

#include "Laguerre.h"

float CubicRealRoot(float i1, float i2, float i3, float i4);

class MAPNoise
{
public:
    MAPNoise(float iSNR = 1.0f, float iAlpha = 1.1f);
    float GammaPrior(float iTotal, float iNoise);
    float GammaPrior(float iTotal, float iNoise, float iAvNoise);
    float InverseGammaPrior(float iTotal, float iNoise);
    float InverseGammaPrior(float iTotal, float iNoise, float iAvNoise);
    float MagGammaPrior(float iTotal, float iNoise);

    void SetAlpha(float iAlpha)
    {
        a = iAlpha;
    }

    void SetSNR(float iSNR)
    {
        w = iSNR;
    }

private:

    // Use short variable names for simplicity
    float w;
    float a;

    Laguerre mLaguerre;
    std::vector<float> mPoly;
};

#endif /* MAPNOISE_H */
