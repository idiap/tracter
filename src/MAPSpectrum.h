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
                const char* iObjectName = "MAPSpectrum");

protected:
    PluginObject* GetInput(int iInput);
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    Plugin<float>* mInput1;
    Plugin<float>* mInput2;
    MAPNoise mMAPNoise;
    bool mGamma;
    bool mAverage;
};

#endif /* MAPSPECTRUM_H */
