/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MAPSPECTRUM_H
#define MAPSPECTRUM_H

#include "CachedPlugin.h"
#include "MAPNoise.h"

/**
 * MAP spectral estimator
 */
class MAPSpectrum : public CachedPlugin<float>
{
public:
    MAPSpectrum(Plugin<float>* iInput1, Plugin<float>* iInput2,
                Plugin<float>* iChannelInput = 0,
                const char* iObjectName = "MAPSpectrum");

protected:
    PluginObject* GetInput(int iInput);
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    Plugin<float>* mInput1;
    Plugin<float>* mInput2;
    Plugin<float>* mChannelInput;
    MAPNoise mMAPNoise;
    int mEstimator;
    float mAlpha;
    float mBeta;
    float mDelta;
    float mSNR;
    std::vector<float> mNoise;
    Laguerre mLaguerre;

    float Average(float* iArray);
    float InverseGammaFixedMode(float iTotal, float iNoise, float iMode);
    float InverseGammaFixedModeLog(float iTotal, float iNoise, float iMode);
    float InverseGammaMode(float iTotal, float iNoise);
    float InverseGammaModeLog(float iTotal, float iNoise);
    float IncompleteGammaLog(float iTotal, float iNoise);
    float IncompleteGammaMag(float iTotal, float iNoise);
    float IncompleteGammaMagLog(float iTotal, float iNoise);
};

#endif /* MAPSPECTRUM_H */
