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

TTracter::ASRFactory::ASRFactory(const char* iObjectName)
{
    mObjectName = iObjectName;

    // List all available front-ends
    mFrontend["Basic"] = &TTracter::ASRFactory::BasicFrontend;
}

Plugin<float>* TTracter::ASRFactory::Frontend(Plugin<float>* iPlugin)
{
    Plugin<float> *plugin = 0;

    const char* frontend = GetEnv("Frontend", "Basic");
    if (mFrontend[frontend])
        return (this->*mFrontend[frontend])(iPlugin);
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
}

Plugin<float>* TTracter::ASRFactory::BasicFrontend(Plugin<float>* iPlugin)
{
    /* Basic signal processing chain */
    ZeroFilter* zf = new ZeroFilter(iPlugin);
    Periodogram* p = new Periodogram(zf);
    MelFilter* mf = new MelFilter(p);
    Cepstrum* c = new Cepstrum(mf);
    return c;
}
