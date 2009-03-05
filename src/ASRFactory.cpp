/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "config.h"

#include "ASRFactory.h"

#include "HTKSource.h"
#include "FileSource.h"
#include "StreamSocketSource.h"

#ifdef HAVE_ALSA
# include "ALSASource.h"
#endif

#ifdef HAVE_SNDFILE
# include "SndFileSource.h"
#endif

#include "Normalise.h"
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
#include "Frame.h"
#include "LPCepstrum.h"

#ifdef HAVE_LIBRESAMPLE
# include "Resample.h"
#endif

#ifdef HAVE_HTKLIB
# include "HCopyWrapper.h"
# include "HTKLibSource.h"
#endif

#ifdef HAVE_BSAPI
# include "BSAPITransform.h"
# include "BSAPIFrontEnd.h"
# include "BSAPIFilterBank.h"
# include "BSAPIFastVTLN.h"
#endif

#ifdef HAVE_TORCH3
# include "MLP.h"
# include "MLPVAD.h"
# undef real
#endif

#include "Energy.h"
#include "ModulationVAD.h"
#include "VADGate.h"

Tracter::ASRFactory::ASRFactory(const char* iObjectName)
{
    mObjectName = iObjectName;

    // List all sources
    mSource["File"] = &Tracter::ASRFactory::fileSource;
#ifdef HAVE_HTKLIB
    mSource["HTKLib"] = &Tracter::ASRFactory::htkLibSource;
#endif
    mSource["Socket"] = &Tracter::ASRFactory::socketSource;
    mSource["HTK"] = &Tracter::ASRFactory::htkSource;
#ifdef HAVE_ALSA
    mSource["ALSA"] = &Tracter::ASRFactory::alsaSource;
#endif
#ifdef HAVE_SNDFILE
    mSource["SndFile"] = &Tracter::ASRFactory::sndFileSource;
#endif

    // List all available front-ends
    mFrontend["Null"] = &Tracter::ASRFactory::nullFrontend;
    mFrontend["Basic"] = &Tracter::ASRFactory::basicFrontend;
    mFrontend["BasicVAD"] = &Tracter::ASRFactory::basicVADFrontend;
    mFrontend["PLP"] = &Tracter::ASRFactory::plpFrontend;

#ifdef HAVE_TORCH3
    mFrontend["BasicMLPVAD"] = &Tracter::ASRFactory::basicMLPVADFrontend;
    mFrontend["MLPVAD"] = &Tracter::ASRFactory::mlpvadFrontend;
#endif

#ifdef HAVE_HTKLIB
    mFrontend["HTK"] = &Tracter::ASRFactory::htkFrontend;
#endif

#ifdef HAVE_BSAPI
    mFrontend["PLPPosterior"] = &Tracter::ASRFactory::plpPosteriorFrontend;
#endif

    // THIS SHOULD NOT BE HERE.  JUST FOR THE REVIEW
    mSpeakerIDSource = 0;
    const char* sidHost = GetEnv("SpeakerIDHost", (char*)0);
    if (sidHost)
    {
        mSpeakerIDSource = new SpeakerIDSocketSource();
        mSpeakerIDSource->Open(sidHost);
    }
}

/**
 * Instantiates a Source based on the ASRFactory_Source configuration
 * variable.
 */
Tracter::Plugin<float>* Tracter::ASRFactory::CreateSource(
    Source*& iSource //< Returns the actual source component
)
{
    Plugin<float> *plugin = 0;

    const char* source = GetEnv("Source", "File");
    if (mSource[source])
        plugin = (this->*mSource[source])(iSource);
    else
        throw Exception("ASRFactory: Unknown source %s\n", source);

#ifdef HAVE_LIBRESAMPLE
    // Not sure if here is the right place...
    if (GetEnv("Resample", 0))
        plugin = new Resample(plugin);
#endif

    return plugin;
}

/**
 * Instantiates a front-end based on the ASRFactory_Frontend
 * configuration variable.
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::CreateFrontend(Plugin<float>* iPlugin)
{
    Plugin<float> *plugin = 0;

    const char* frontend = GetEnv("Frontend", "Basic");
    if (mFrontend[frontend])
        plugin = (this->*mFrontend[frontend])(iPlugin);
    else
        throw Exception("ASRFactory: Unknown frontend %s\n", frontend);

    return plugin;
}

/**
 * Instantiates a FileSource<short> followed by a Normalise component
 */
Tracter::Plugin<float>* Tracter::ASRFactory::fileSource(Source*& iSource)
{
    FileSource<short>* s = new FileSource<short>();
    Normalise* n = new Normalise(s);
    iSource = s;
    return n;
}

#ifdef HAVE_SNDFILE
/**
 * Instantiates a SndFileSource component
 */
Tracter::Plugin<float>* Tracter::ASRFactory::sndFileSource(Source*& iSource)
{
    SndFileSource* s = new SndFileSource();
    iSource = s;
    return s;
}
#endif

#ifdef HAVE_ALSA
/**
 * Instantiates an ALSASource followed by a Normalise component
 */
Tracter::Plugin<float>* Tracter::ASRFactory::alsaSource(Source*& iSource)
{
    ALSASource* s = new ALSASource();
    Normalise* n = new Normalise(s);
    iSource = s;
    return n;
}
#endif

/**
 * Instantiates a StreamSocketSource component
 */
Tracter::Plugin<float>* Tracter::ASRFactory::socketSource(Source*& iSource)
{
    StreamSocketSource* s = new StreamSocketSource();
    iSource = s;
    return s;
}

#ifdef HAVE_HTKLIB
/**
 * Instantiates an HTKLibSource component
 */
Tracter::Plugin<float>* Tracter::ASRFactory::htkLibSource(Source*& iSource)
{
    HTKLibSource * s = new HTKLibSource();
    iSource = s;
    return s;
}
#endif

/**
 * Instantiates an HTKSource component
 */
Tracter::Plugin<float>* Tracter::ASRFactory::htkSource(Source*& iSource)
{
    HTKSource* s = new HTKSource();
    iSource = s;
    return s;
}

/**
 * Instantiates an arbitrary graph of Delta components
 */
Tracter::Plugin<float>* Tracter::ASRFactory::deltas(Plugin<float>* iPlugin)
{
    Plugin<float>* plugin = iPlugin;
    int deltaOrder = GetEnv("DeltaOrder", 0);
    if (deltaOrder > 0)
    {
        Concatenate* c = new Concatenate();
        c->Add(plugin);
        for (int i=0; i<deltaOrder; i++)
        {
            // For the moment, there is a bug where the plugins don't
            // copy this string, so the pointer becomes invalid.
            //char str[10];
            //sprintf(str, "Delta%d", i+1);
            //Delta* d = new Delta(plugin, str); 
            Delta* d = new Delta(plugin);
            c->Add(d);
            plugin = d;
        }
        plugin = c;
    }
    return plugin;
}

/**
 * Instantiates a Mean component with associated Subtract
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::normaliseMean(Plugin<float>* iPlugin)
{
    Plugin<float>* plugin = iPlugin;
    bool cmn = GetEnv("NormaliseMean", 1);
    if (cmn)
    {
        Mean* m = new Mean(iPlugin);
        Subtract* s = new Subtract(iPlugin, m);
        plugin = s;
    }
    return plugin;
}

/**
 * Instantiates a Variance component with associated Divide
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::normaliseVariance(Plugin<float>* iPlugin)
{
    Plugin<float>* plugin = iPlugin;
    bool cvn = GetEnv("NormaliseVariance", 0);
    if (cvn)
    {
        Variance* v = new Variance(iPlugin);
        Divide* d = new Divide(iPlugin, v);
        plugin = d;
    }
    return plugin;
}

/**
 * Does nothing, but allows a "null" frontend, effectively allowing a
 * direct connection to the source.
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::nullFrontend(Plugin<float>* iPlugin)
{
    return iPlugin;
}

/**
 * Instantiates a basic MFCC frontend.
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::basicFrontend(Plugin<float>* iPlugin)
{
    Plugin<float>* p = iPlugin;
    p = new ZeroFilter(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    return p;
}

/**
 * Instantiates a basic MFCC frontend with ModulationVAD and VADGate
 * components.
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::basicVADFrontend(Plugin<float>* iPlugin)
{
    /* Basic signal processing chain */
    Plugin<float>* p = iPlugin;
    p = new ZeroFilter(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);

    /* VAD */
    Plugin<float>* v = iPlugin;
    v = new Energy(v);
    ModulationVAD* mv = new ModulationVAD(v);
    v = new VADGate(p, mv);

    return v;
}

#ifdef HAVE_TORCH3
/**
 * Instantiates a basic MFCC frontend with MLPVAD and VADGate
 * components.
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::basicMLPVADFrontend(Plugin<float>* iPlugin)
{
    /* Basic signal processing chain */
    Plugin<float>* p = iPlugin;
    p = new ZeroFilter(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);

    /* VAD - works on the "basic" features */
    Plugin<float>* v = new MLP(p);
    MLPVAD* mv = new MLPVAD(v);
    p = new VADGate(p, mv);

    return p;
}

/**
 * Instantiates MLPVAD and VADGate components on the assumption that
 * the source is providing suitable features directly.
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::mlpvadFrontend(Plugin<float>* iPlugin)
{
    /* VAD working on features */
    Plugin<float>* p = iPlugin;
    Plugin<float>* v = new MLP(p);
    MLPVAD* mv = new MLPVAD(v);
    p = new VADGate(p, mv);

    return p;
}
#endif

/**
 * Instantiates a PLP frontend.
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::plpFrontend(Plugin<float>* iPlugin)
{
    Plugin<float>* p = iPlugin;
    p = new ZeroFilter(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new LPCepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    return p;
}

#ifdef HAVE_HTKLIB
/**
 * Instantiates an HCopyWrapper component
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::htkFrontend(Plugin<float>* iPlugin)
{
    Plugin<float>* p = iPlugin;
    p = new HCopyWrapper(p);
    return p;
}
#endif

#ifdef HAVE_BSAPI
/**
 * Instantiates BSAPI components with both standard and posterior based
 * features.
 */
Tracter::Plugin<float>*
Tracter::ASRFactory::plpPosteriorFrontend(Plugin<float>* iPlugin)
{
    Plugin<float>* p  = iPlugin;

    // Framed version of the input for BSAPI
    Plugin<float>* f = new Frame(p);

#ifdef HAVE_TORCH3
    // MLP based VAD
    p = new BSAPIFrontEnd(f, "PLPFrontEnd");
    Mean* mlpm = new Mean(p);
    p = new Subtract(p, mlpm);
    Variance* mlpv = new Variance(p);
    p = new Divide(p, mlpv);
    p = new MLP(p);
    MLPVAD* m = new MLPVAD(p);
    p = new VADGate(f, m);
#else
    // Energy based VAD (Energy will do the framing)
    p = new Energy(p);
    ModulationVAD* m = new ModulationVAD(p);
    p = new VADGate(f, m);
#endif

    // VTLN PLP
    Plugin<float>* wf = new BSAPIFastVTLN(p);

    // NN Front End
    Plugin<float>* nn;
    nn = new BSAPIFilterBank(p, wf);
    Mean* nnm = new Mean(nn);
    nn = new Subtract(nn, nnm);
    Variance* nnv = new Variance(nn);
    nn = new Divide(nn, nnv);
    nn = new BSAPITransform(nn, "NNTransform");

    // PLP HLDA FrontEnd
    Plugin<float>* plp;
    plp = new BSAPIFrontEnd(p, wf, "PLPHLDAFrontEnd");
    Mean* plpm = new Mean(plp);
    plp = new Subtract(plp, plpm);
    plp = new BSAPITransform(plp, "DATTransform");
    Variance* plpv = new Variance(plp, "PLPVariance");
    plp = new Divide(plp, plpv);
    plp = new BSAPITransform(plp, "HLDATransform");

    // Concatenation
    Concatenate* c = new Concatenate();
    c->Add(plp);
    c->Add(nn);
    Mean* cm = new Mean(c);
    p = new Subtract(c, cm);
    Variance* cv = new Variance(p, "CatVariance");
    p = new Divide(p, cv);

    // Done
    return p;
}
#endif


// THIS SHOULD NOT BE HERE.  JUST FOR THE REVIEW.
Tracter::Plugin<float>*
Tracter::ASRFactory::GetSpeakerIDSource()
{
    if (mSpeakerIDSource)
        return mSpeakerIDSource;

    return 0;
}
