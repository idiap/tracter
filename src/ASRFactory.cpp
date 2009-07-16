/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "config.h"

#include "ASRFactory.h"

#include "HTKSource.h"
#include "LNASource.h"
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

#ifdef HAVE_SPTK
# include "MCep.h"
#endif

#include "Energy.h"
#include "VADGate.h"
#include "Modulation.h"
#include "NoiseVAD.h"

#include "LinearTransform.h"

Tracter::ASRFactory::ASRFactory(const char* iObjectName)
{
    mObjectName = iObjectName;

    // List all sources
    RegisterSource(new FileSourceFactory);
#ifdef HAVE_HTKLIB
    RegisterSource(new HTKLibSourceFactory);
#endif
    RegisterSource(new StreamSocketSourceFactory);
    RegisterSource(new HTKSourceFactory);
    RegisterSource(new LNASourceFactory);
#ifdef HAVE_ALSA
    RegisterSource(new ALSASourceFactory);
#endif
#ifdef HAVE_SNDFILE
    RegisterSource(new SndFileSourceFactory);
#endif

    // List all available front-ends
    RegisterFrontend(new NullGraphFactory);
    RegisterFrontend(new CMVNGraphFactory);
    RegisterFrontend(new BasicGraphFactory);
    RegisterFrontend(new BasicVADGraphFactory);
    RegisterFrontend(new PLPGraphFactory);

#ifdef HAVE_TORCH3
    RegisterFrontend(new BasicMLPVADGraphFactory);
    RegisterFrontend(new MLPVADGraphFactory);
#endif

#ifdef HAVE_HTKLIB
    RegisterFrontend(new HTKGraphFactory);
#endif

#ifdef HAVE_BSAPI
    RegisterFrontend(new PLPPosteriorGraphFactory);
    RegisterFrontend(new PLPvtlnGraphFactory);
#endif

#ifdef HAVE_SPTK
    RegisterFrontend(new MCepGraphFactory);
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

Tracter::ASRFactory::~ASRFactory() throw ()
{
    std::map<std::string, SourceFactory*>::iterator s;
    for (s = mSource.begin(); s != mSource.end(); ++s)
    {
        delete s->second;
    }

    std::map<std::string, GraphFactory*>::iterator g;
    for (g = mFrontend.begin(); g != mFrontend.end(); ++g)
    {
        delete g->second;
    }
}

/**
 * Instantiates a Source based on the ASRFactory_Source configuration
 * variable.
 */
Tracter::Plugin<float>* Tracter::ASRFactory::CreateSource(
    ISource*& iSource //< Returns the actual source component
)
{
    Plugin<float> *plugin = 0;

    const char* source = GetEnv("Source", "File");
    if (mSource[source])
        plugin = mSource[source]->Create(iSource);
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
        plugin = mFrontend[frontend]->Create(iPlugin);
    else
        throw Exception("ASRFactory: Unknown frontend %s\n", frontend);

    return plugin;
}

/**
 * Instantiates a FileSource<short> followed by a Normalise component
 */
Tracter::Plugin<float>*
Tracter::FileSourceFactory::Create(ISource*& iSource)
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
Tracter::Plugin<float>*
Tracter::SndFileSourceFactory::Create(ISource*& iSource)
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
Tracter::Plugin<float>*
Tracter::ALSASourceFactory::Create(ISource*& iSource)
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
Tracter::Plugin<float>*
Tracter::StreamSocketSourceFactory::Create(ISource*& iSource)
{
    StreamSocketSource* s = new StreamSocketSource();
    iSource = s;
    return s;
}

#ifdef HAVE_HTKLIB
/**
 * Instantiates an HTKLibSource component
 */
Tracter::Plugin<float>*
Tracter::HTKLibSourceFactory::Create(ISource*& iSource)
{
    HTKLibSource * s = new HTKLibSource();
    iSource = s;
    return s;
}
#endif

/**
 * Instantiates an HTKSource component
 */
Tracter::Plugin<float>*
Tracter::HTKSourceFactory::Create(ISource*& iSource)
{
    HTKSource* s = new HTKSource();
    iSource = s;
    return s;
}

/**
 * Instantiates an LNASource component
 */
Tracter::Plugin<float>*
Tracter::LNASourceFactory::Create(ISource*& iSource)
{
    LNASource* s = new LNASource();
    iSource = s;
    return s;
}

/**
 * Instantiates an arbitrary graph of Delta components
 */
Tracter::Plugin<float>*
Tracter::GraphFactory::deltas(Plugin<float>* iPlugin)
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
Tracter::GraphFactory::normaliseMean(Plugin<float>* iPlugin)
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
Tracter::GraphFactory::normaliseVariance(Plugin<float>* iPlugin)
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
Tracter::NullGraphFactory::Create(Plugin<float>* iPlugin)
{
    return iPlugin;
}

/**
 * Does nothing other than add CMVN and deltas if necessary.  Requires
 * a feature level source.
 */
Tracter::Plugin<float>*
Tracter::CMVNGraphFactory::Create(Plugin<float>* iPlugin)
{
    Plugin<float>* p = iPlugin;
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    p = new LinearTransform(p);
    return p;
}

/**
 * Instantiates a basic MFCC frontend.
 */
Tracter::Plugin<float>*
Tracter::BasicGraphFactory::Create(Plugin<float>* iPlugin)
{
    Plugin<float>* p = iPlugin;
    p = new ZeroFilter(p);
    p = new Frame(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    return p;
}

/**
 * Instantiates a basic MFCC frontend with energy based VAD and
 * VADGate components.
 */
Tracter::Plugin<float>*
Tracter::BasicVADGraphFactory::Create(Plugin<float>* iPlugin)
{
    /* Basic signal processing chain */
    Plugin<float>* p = iPlugin;
    p = new ZeroFilter(p);
    p = new Frame(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);

    /* VAD */
    Plugin<float>* v = iPlugin;
    v = new Frame(v);
    v = new Energy(v);
    Modulation* m = new Modulation(v);
    NoiseVAD* mv = new NoiseVAD(m, v);
    v = new VADGate(p, mv);

    return v;
}

#ifdef HAVE_TORCH3
/**
 * Instantiates a basic MFCC frontend with MLPVAD and VADGate
 * components.
 */
Tracter::Plugin<float>*
Tracter::BasicMLPVADGraphFactory::Create(Plugin<float>* iPlugin)
{
    /* Basic signal processing chain */
    Plugin<float>* p = iPlugin;
    p = new ZeroFilter(p);
    p = new Frame(p);
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
Tracter::MLPVADGraphFactory::Create(Plugin<float>* iPlugin)
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
Tracter::PLPGraphFactory::Create(Plugin<float>* iPlugin)
{
    Plugin<float>* p = iPlugin;
    p = new ZeroFilter(p);
    p = new Frame(p);
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
Tracter::HTKGraphFactory::Create(Plugin<float>* iPlugin)
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
Tracter::PLPPosteriorGraphFactory::Create(Plugin<float>* iPlugin)
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
    // Energy based VAD
    p = new Frame(p);
    p = new Energy(p);
    Modulation* m = new Modulation(p);
    NoiseVAD* mv = new NoiseVAD(m, p)
    p = new VADGate(f, mv);
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
    return p;    // Returns the concatenated VPLP , posterior features
}

/**
 * Instantiates BSAPI components with both standard VTLN PLP
 * features.
 */
Tracter::Plugin<float>*
Tracter::PLPvtlnGraphFactory::Create(Plugin<float>* iPlugin)
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
    // Energy based VAD
    p = new Frame(p);
    p = new Energy(p);
    Modulation* m = new Modulation(p);
    NoiseVAD* mv = new NoiseVAD(m, p)
    p = new VADGate(f, mv);
#endif

    // VTLN PLP
    Plugin<float>* wf = new BSAPIFastVTLN(p);

    // PLP HLDA FrontEnd
    Plugin<float>* plp;
    plp = new BSAPIFrontEnd(p, wf, "PLPHLDAFrontEnd");
    Mean* plpm = new Mean(plp);
    plp = new Subtract(plp, plpm);
    plp = new BSAPITransform(plp, "DATTransform");
//  Variance* plpv = new Variance(plp, "PLPVariance");
//  plp = new Divide(plp, plpv);
    plp = new BSAPITransform(plp, "HLDATransform");

    // Done
    return plp;  // Returns only the VTLN PLPs
}
#endif

#ifdef HAVE_SPTK
/**
 * Instantiates a SPTK based mcep frontend.
 */
Tracter::Plugin<float>*
Tracter::MCepGraphFactory::Create(Plugin<float>* iPlugin)
{
    Plugin<float>* p = iPlugin;
    p = new Frame(p);
    p = new Periodogram(p);
    p = new MCep(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
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
