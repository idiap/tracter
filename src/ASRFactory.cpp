/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ASRFactory.h"
#include "Mean.h"
#include "Subtract.h"
#include "Concatenate.h"
#include "Delta.h"
#include "Variance.h"
#include "Divide.h"
#include "ZeroFilter.h"
#include "Periodogram.h"
#include "MelFilter.h"
#include "Cepstrum.h"

#include "PLP.h"
#include "WarpedPeriodogram.h"
#include "Noise.h"
#include "GeometricNoise.h"
#include "MAPSpectrum.h"

Tracter::ASRFactory::ASRFactory(const char* iObjectName)
{
    mObjectName = iObjectName;

    // List all available front-ends
    mFrontend["Basic"] = &Tracter::ASRFactory::BasicFrontend;
    mFrontend["Noise"] = &Tracter::ASRFactory::NoiseFrontend;
    mFrontend["PLP"] = &Tracter::ASRFactory::PLPFrontend;
    mFrontend["Complex"] = &Tracter::ASRFactory::ComplexFrontend;
}

Plugin<float>* Tracter::ASRFactory::Frontend(Plugin<float>* iPlugin)
{
    Plugin<float> *plugin = 0;

    const char* frontend = GetEnv("Frontend", "Basic");
    if (mFrontend[frontend])
        plugin = (this->*mFrontend[frontend])(iPlugin);
    else
    {
        printf("ASRFactory: Unknown frontend %s\n", frontend);
        exit(EXIT_FAILURE);
    }

    bool cmn = GetEnv("NormaliseMean", 1);
    if (cmn)
    {
        Mean* m = new Mean(plugin);
        Subtract* s = new Subtract(plugin, m);
        plugin = s;
    }

    int deltaOrder = GetEnv("DeltaOrder", 0);
    if (deltaOrder > 0)
    {
        Concatenate* c = new Concatenate();
        c->Add(plugin);
        for (int i=0; i<deltaOrder; i++)
        {
            Delta* d = new Delta(plugin);
            c->Add(d);
            plugin = d;
        }
        plugin = c;
    }

    bool cvn = GetEnv("NormaliseVariance", 0);
    if (cvn)
    {
        Variance* v = new Variance(plugin);
        Divide* d = new Divide(plugin, v);
        plugin = d;
    }

    return plugin;
}

Plugin<float>* Tracter::ASRFactory::BasicFrontend(Plugin<float>* iPlugin)
{
    /* Basic signal processing chain */
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    Periodogram* p = new Periodogram(zf);
    MelFilter* mf = new MelFilter(p);
    Cepstrum* c = new Cepstrum(mf);
    return c;
}

Plugin<float>* Tracter::ASRFactory::PLPFrontend(Plugin<float>* iPlugin)
{
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    Periodogram* p = new Periodogram(zf);
    MelFilter* mf = new MelFilter(p);
    PLP* l = new PLP(mf);
    return l;
}

Plugin<float>* Tracter::ASRFactory::ComplexFrontend(Plugin<float>* iPlugin)
{
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    WarpedPeriodogram* p = new WarpedPeriodogram(zf);
    Cepstrum* c = new Cepstrum(p);
    return c;
}

Plugin<float>* Tracter::ASRFactory::NoiseFrontend(Plugin<float>* iPlugin)
{
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    Periodogram* p = new Periodogram(zf);
    GeometricNoise* gn = new GeometricNoise(p);
    Noise* nn = new Noise(p);
    MAPSpectrum *mp = new MAPSpectrum(p, nn, gn);
    MelFilter* mf = new MelFilter(mp);
    Cepstrum* c = new Cepstrum(mf);
    return c;
}
