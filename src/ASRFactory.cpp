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

#include "Pixmap.h"

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
Tracter::Component<float>* Tracter::ASRFactory::CreateSource(
    ISource*& iSource //< Returns the actual source component
)
{
    Component<float> *component = 0;

    const char* source = GetEnv("Source", "File");
    if (mSource[source])
        component = mSource[source]->Create(iSource);
    else
        throw Exception("ASRFactory: Unknown source %s\n", source);

#ifdef HAVE_LIBRESAMPLE
    // Not sure if here is the right place...
    if (GetEnv("Resample", 0))
        component = new Resample(component);
#endif

    return component;
}

/**
 * Instantiates a front-end based on the ASRFactory_Frontend
 * configuration variable.
 */
Tracter::Component<float>*
Tracter::ASRFactory::CreateFrontend(Component<float>* iComponent)
{
    Component<float> *component = 0;

    const char* frontend = GetEnv("Frontend", "Null");
    if (mFrontend[frontend])
        component = mFrontend[frontend]->Create(iComponent);
    else
        throw Exception("ASRFactory: Unknown frontend %s\n", frontend);

    return component;
}

/**
 * Instantiates a FileSource<short> followed by a Normalise component
 */
Tracter::Component<float>*
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
Tracter::Component<float>*
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
Tracter::Component<float>*
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
Tracter::Component<float>*
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
Tracter::Component<float>*
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
Tracter::Component<float>*
Tracter::HTKSourceFactory::Create(ISource*& iSource)
{
    HTKSource* s = new HTKSource();
    iSource = s;
    return s;
}

/**
 * Instantiates an LNASource component
 */
Tracter::Component<float>*
Tracter::LNASourceFactory::Create(ISource*& iSource)
{
    LNASource* s = new LNASource();
    iSource = s;
    return s;
}

/**
 * Instantiates an arbitrary graph of Delta components
 */
Tracter::Component<float>*
Tracter::GraphFactory::deltas(Component<float>* iComponent)
{
    Component<float>* component = iComponent;
    int deltaOrder = GetEnv("DeltaOrder", 0);
    if (deltaOrder > 0)
    {
        Concatenate* c = new Concatenate();
        c->Add(component);
        for (int i=0; i<deltaOrder; i++)
        {
            // For the moment, there is a bug where the components don't
            // copy this string, so the pointer becomes invalid.
            //char str[10];
            //sprintf(str, "Delta%d", i+1);
            //Delta* d = new Delta(component, str); 
            Delta* d = new Delta(component);
            c->Add(d);
            component = d;
        }
        component = c;
    }
    return component;
}

/**
 * Instantiates a Mean component with associated Subtract
 */
Tracter::Component<float>*
Tracter::GraphFactory::normaliseMean(Component<float>* iComponent)
{
    Component<float>* component = iComponent;
    bool cmn = GetEnv("NormaliseMean", 1);
    if (cmn)
    {
        Mean* m = new Mean(iComponent);
        Subtract* s = new Subtract(iComponent, m);
        component = s;
    }
    return component;
}

/**
 * Instantiates a Variance component with associated Divide
 */
Tracter::Component<float>*
Tracter::GraphFactory::normaliseVariance(Component<float>* iComponent)
{
    Component<float>* component = iComponent;
    bool cvn = GetEnv("NormaliseVariance", 0);
    if (cvn)
    {
        Variance* v = new Variance(iComponent);
        Divide* d = new Divide(iComponent, v);
        component = d;
    }
    return component;
}

/**
 * Does nothing, but allows a "null" frontend, effectively allowing a
 * direct connection to the source.
 */
Tracter::Component<float>*
Tracter::NullGraphFactory::Create(Component<float>* iComponent)
{
    return iComponent;
}

/**
 * Does nothing other than add CMVN and deltas if necessary.  Requires
 * a feature level source.
 */
Tracter::Component<float>*
Tracter::CMVNGraphFactory::Create(Component<float>* iComponent)
{
    Component<float>* p = iComponent;
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    p = new LinearTransform(p);
    return p;
}

/**
 * Instantiates a basic MFCC frontend.
 */
Tracter::Component<float>*
Tracter::BasicGraphFactory::Create(Component<float>* iComponent)
{
    Component<float>* p = iComponent;
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
Tracter::Component<float>*
Tracter::BasicVADGraphFactory::Create(Component<float>* iComponent)
{
    /* Basic signal processing chain */
    Component<float>* p = iComponent;
    p = new ZeroFilter(p);
    p = new Frame(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);

    /* VAD */
    Component<float>* v = iComponent;
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
Tracter::Component<float>*
Tracter::BasicMLPVADGraphFactory::Create(Component<float>* iComponent)
{
    /* Basic signal processing chain */
    Component<float>* p = iComponent;
    p = new ZeroFilter(p);
    p = new Frame(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);

    /* VAD - works on the "basic" features */
    Component<float>* v = new MLP(p);
    MLPVAD* mv = new MLPVAD(v);
    p = new VADGate(p, mv);

    return p;
}

/**
 * Instantiates MLPVAD and VADGate components on the assumption that
 * the source is providing suitable features directly.
 */
Tracter::Component<float>*
Tracter::MLPVADGraphFactory::Create(Component<float>* iComponent)
{
    /* VAD working on features */
    Component<float>* p = iComponent;
    Component<float>* v = new MLP(p);
    MLPVAD* mv = new MLPVAD(v);
    p = new VADGate(p, mv);

    return p;
}
#endif

/**
 * Instantiates a PLP frontend.
 */
Tracter::Component<float>*
Tracter::PLPGraphFactory::Create(Component<float>* iComponent)
{
    Component<float>* p = iComponent;
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
Tracter::Component<float>*
Tracter::HTKGraphFactory::Create(Component<float>* iComponent)
{
    Component<float>* p = iComponent;
    p = new HCopyWrapper(p);
    return p;
}
#endif

#ifdef HAVE_BSAPI
/**
 * Instantiates BSAPI components with both standard and posterior based
 * features.
 */
Tracter::Component<float>*
Tracter::PLPPosteriorGraphFactory::Create(Component<float>* iComponent)
{
    Component<float>* p  = iComponent;

    // Framed version of the input for BSAPI
    Component<float>* f = new Frame(p);

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
    Component<float>* wf = new BSAPIFastVTLN(p);

    // NN Front End
    Component<float>* nn;
    nn = new BSAPIFilterBank(p, wf);
    Mean* nnm = new Mean(nn);
    nn = new Subtract(nn, nnm);
    Variance* nnv = new Variance(nn);
    nn = new Divide(nn, nnv);
    nn = new BSAPITransform(nn, "NNTransform");

    // PLP HLDA FrontEnd
    Component<float>* plp;
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
Tracter::Component<float>*
Tracter::PLPvtlnGraphFactory::Create(Component<float>* iComponent)
{
    Component<float>* p  = iComponent;

    // Framed version of the input for BSAPI
    Component<float>* f = new Frame(p);

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
    Component<float>* wf = new BSAPIFastVTLN(p);

    // PLP HLDA FrontEnd
    Component<float>* plp;
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
Tracter::Component<float>*
Tracter::MCepGraphFactory::Create(Component<float>* iComponent)
{
    Component<float>* p = iComponent;
    p = new Frame(p);
    p = new Periodogram(p);
    p = new MCep(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    return p;
}
#endif
