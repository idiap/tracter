/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

/*
 * The ASR factory is a kind of dumping ground for all sorts of ASR
 * experiments.  The Basic and BasicVAD graphs are pretty useful.
 */

#include "ASRFactory.h"

#include "EnergyNorm.h"
#include "Log.h"
#include "Select.h"
#include "ViterbiVAD.h"
#include "ViterbiVADGate.h"

#include "HTKSource.h"
#include "LNASource.h"
#include "FileSource.h"
#include "StreamSocketSource.h"

#ifdef HAVE_ALSA
# include "ALSASource.h"
#endif

#ifdef HAVE_RTAUDIO
# include "RtAudioSource.h"
#endif

#ifdef HAVE_PULSEAUDIO
# include "PulseAudioSource.h"
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

#include "Resample.h"

#ifdef HAVE_SPTK
# include "MCep.h"
#endif

#ifdef HAVE_LIBSSP
# include "CochlearFilter.h"
# include "CochlearFrame.h"
#endif

#include "Pixmap.h"

#include "Energy.h"
#include "VADGate.h"
#include "Modulation.h"
#include "NoiseVAD.h"

#include "LinearTransform.h"

#include "SNRSpectrum.h"
#include "Minima.h"
#include "TransverseFilter.h"

#include "Comparator.h"
#include "TimedLatch.h"
#include "Gate.h"
#include "BoolToFloat.h"

Tracter::ASRFactory::ASRFactory(const char* iObjectName)
{
    objectName(iObjectName);

    // List all sources
    registerSource(new FileSourceFactory);
    registerSource(new StreamSocketSourceFactory);
    registerSource(new HTKSourceFactory);
    registerSource(new LNASourceFactory);
#ifdef HAVE_ALSA
    registerSource(new ALSASourceFactory);
#endif
#ifdef HAVE_RTAUDIO
    registerSource(new RtAudioSourceFactory);
#endif
#ifdef HAVE_PULSEAUDIO
    registerSource(new PulseAudioSourceFactory);
#endif
#ifdef HAVE_SNDFILE
    registerSource(new SndFileSourceFactory);
#endif

    // List all available front-ends
    registerFrontend(new NullGraphFactory);
    registerFrontend(new CMVNGraphFactory);
    registerFrontend(new BasicGraphFactory);
    registerFrontend(new BasicSpeechDetGraphFactory);
    registerFrontend(new BasicVADGraphFactory);
    registerFrontend(new PLPGraphFactory);
    registerFrontend(new PLPVADGraphFactory);

#ifdef HAVE_SPTK
    registerFrontend(new MCepGraphFactory);
#endif

#ifdef HAVE_LIBSSP
    registerFrontend(new CochlearGraphFactory);
    registerFrontend(new CochlearSNRGraphFactory);
#endif

    registerFrontend(new SNRGraphFactory);
}

Tracter::ASRFactory::~ASRFactory()
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
Tracter::Component<float>* Tracter::ASRFactory::createSource(
    ISource*& iSource //< Returns the actual source component
)
{
    Component<float> *component = 0;

    const char* source = config("Source", "File");
    if (mSource[source])
        component = mSource[source]->create(iSource);
    else
        throw Exception("ASRFactory: Unknown source %s\n", source);

#ifdef HAVE_RESAMPLE
    // Not sure if here is the right place...
    if (config("Resample", 0))
        component = new Resample(component);
#endif

    return component;
}

/**
 * Instantiates a front-end based on the ASRFactory_Frontend
 * configuration variable.
 */
Tracter::Component<float>*
Tracter::ASRFactory::createFrontend(Component<float>* iComponent)
{
    Component<float> *component = 0;

    const char* frontend = config("Frontend", "Null");
    if (mFrontend[frontend])
        component = mFrontend[frontend]->create(iComponent);
    else
        throw Exception("ASRFactory: Unknown frontend %s\n", frontend);

    return component;
}

/**
 * Instantiates a FileSource<short> followed by a Normalise component
 */
Tracter::Component<float>*
Tracter::FileSourceFactory::create(ISource*& iSource)
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
Tracter::SndFileSourceFactory::create(ISource*& iSource)
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
Tracter::ALSASourceFactory::create(ISource*& iSource)
{
    ALSASource* s = new ALSASource();
    Normalise* n = new Normalise(s);
    iSource = s;
    return n;
}
#endif

#ifdef HAVE_RTAUDIO
/**
 * Instantiates an RtAudioSource followed by a Normalise component
 */
Tracter::Component<float>*
Tracter::RtAudioSourceFactory::create(ISource*& iSource)
{
    RtAudioSource* s = new RtAudioSource();
    iSource = s;
    return s;
}
#endif

#ifdef HAVE_PULSEAUDIO
/**
 * Instantiates an PulseAudioSource followed by a Normalise component
 */
Tracter::Component<float>*
Tracter::PulseAudioSourceFactory::create(ISource*& iSource)
{
    PulseAudioSource* s = new PulseAudioSource();
    iSource = s;
    return s;
}
#endif

/**
 * Instantiates a StreamSocketSource component
 */
Tracter::Component<float>*
Tracter::StreamSocketSourceFactory::create(ISource*& iSource)
{
    StreamSocketSource* s = new StreamSocketSource();
    iSource = s;
    return s;
}

/**
 * Instantiates an HTKSource component
 */
Tracter::Component<float>*
Tracter::HTKSourceFactory::create(ISource*& iSource)
{
    HTKSource* s = new HTKSource();
    iSource = s;
    return s;
}

/**
 * Instantiates an LNASource component
 */
Tracter::Component<float>*
Tracter::LNASourceFactory::create(ISource*& iSource)
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
    int deltaOrder = config("DeltaOrder", 0);
    if (deltaOrder > 0)
    {
        Concatenate* c = new Concatenate();
        c->add(component);
        for (int i=0; i<deltaOrder; i++)
        {
            // For the moment, there is a bug where the components don't
            // copy this string, so the pointer becomes invalid.
            //char str[10];
            //sprintf(str, "Delta%d", i+1);
            //Delta* d = new Delta(component, str); 
            Delta* d = new Delta(component);
            c->add(d);
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
    bool cmn = config("NormaliseMean", 1);
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
    bool cvn = config("NormaliseVariance", 0);
    if (cvn)
    {
        Component<float>* v = new Variance(iComponent);
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
Tracter::NullGraphFactory::create(Component<float>* iComponent)
{
    return iComponent;
}

/**
 * Does nothing other than add CMVN and deltas if necessary.  Requires
 * a feature level source.
 */
Tracter::Component<float>*
Tracter::CMVNGraphFactory::create(Component<float>* iComponent)
{
    Component<float>* p = iComponent;
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);

    // Doesn't really belong, but it's easy to "comment out" behind the option
    if (config("LinearTransform", false))
        p = new LinearTransform(p);
    return p;
}

/**
 * Instantiates a basic MFCC frontend.
 */
Tracter::Component<float>*
Tracter::BasicGraphFactory::create(Component<float>* iComponent)
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
 * Instantiates a basic MFCC frontend with speech/sil detection.
 */
Tracter::Component<float>*
Tracter::BasicSpeechDetGraphFactory::create(Component<float>* iComponent)
{
    // Features pipeline
    Component<float>* p = iComponent;
    p = new ZeroFilter(p);
    p = new Frame(p);
    p = new Periodogram(p);
    p = new MelFilter(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);

    // Minima-based VAD
    Component<float>* v = iComponent;
    v = new Frame(v);
    v = new Energy(v);
    Modulation* m = new Modulation(v);
    Component<float>* n = new Minima(v);
    Component<BoolType>* b = new Comparator(m, n);
    b = new TimedLatch(b);
    Component<float>* f = new BoolToFloat(b);
    
    // Concatenation
    Concatenate* c = new Concatenate();
    c->add(p);
    c->add(f);

    return c;
}

/**
 * Instantiates a basic MFCC frontend with energy based VAD and
 * VADGate components.
 */
Tracter::Component<float>*
Tracter::BasicVADGraphFactory::create(Component<float>* iComponent)
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
    if (!config("MinimaVAD", 0))
    {
        // Old VAD
        NoiseVAD* mv = new NoiseVAD(m, v);
        v = new VADGate(p, mv);
    }
    else
    {
        // New minima-based VAD
        Component<float>* n = new Minima(v);
        Component<BoolType>* b = new Comparator(m, n);
        b = new TimedLatch(b);
        v = new Gate(p, b);
    }

    return v;
}

/**
 * Instantiates a PLP frontend.
 */
Tracter::Component<float>*
Tracter::PLPGraphFactory::create(Component<float>* iComponent)
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

/**
 * Instantiates a PLP frontend with energy based VAD and
 * VADGate components.
 */
Tracter::Component<float>*
Tracter::PLPVADGraphFactory::create(Component<float>* iComponent)
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

    /* VAD */
    Component<float>* v = iComponent;
    v = new Frame(v);
    v = new Energy(v);
    Modulation* m = new Modulation(v);
    if (!config("MinimaVAD", 0))
    {
        // Old VAD
        NoiseVAD* mv = new NoiseVAD(m, v);
        v = new VADGate(p, mv);
    }
    else
    {
        // New minima-based VAD
        Component<float>* n = new Minima(v);
        Component<BoolType>* b = new Comparator(m, n);
        b = new TimedLatch(b);
        v = new Gate(p, b);
    }

    return v;
}

#ifdef HAVE_SPTK
/**
 * Instantiates a SPTK based mcep frontend.
 */
Tracter::Component<float>*
Tracter::MCepGraphFactory::create(Component<float>* iComponent)
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

/**
 * Instantiates a "basic MFCC" frontend with SNR spectral features.
 */
Tracter::Component<float>*
Tracter::SNRGraphFactory::create(Component<float>* iComponent)
{
    bool mel = config("Mel", 0);
    Component<float>* p = iComponent;
    p = new ZeroFilter(p);
    p = new Frame(p);
    p = new Periodogram(p);
    if (mel)
        p = new MelFilter(p);
    Component<float>* m = new Minima(p);
    m = new TransverseFilter(m);
    p = new SNRSpectrum(p, m);
    if (!mel)
        p = new MelFilter(p);
    if (config("PLP", 0))
        p = new LPCepstrum(p);
    else
        p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    return p;
}

#ifdef HAVE_LIBSSP
/**
 * Instantiates a libssp based cochlear frontend.
 */
Tracter::Component<float>*
Tracter::CochlearGraphFactory::create(Component<float>* iComponent)
{
    Component<float>* p = iComponent;
    p = new ZeroFilter(p);
    p = new CochlearFilter(p);
    p = new CochlearFrame(p);
    p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    return p;
}

/**
 * Instantiates a cochlear frontend with SNR spectral features.
 */
Tracter::Component<float>*
Tracter::CochlearSNRGraphFactory::create(Component<float>* iComponent)
{
    Component<float>* p = iComponent;
    p = new ZeroFilter(p);
    p = new CochlearFilter(p);
    p = new CochlearFrame(p);
    Component<float>* m = new Minima(p);
    //m = new TransverseFilter(m);
    p = new SNRSpectrum(p, m);
    if (config("PLP", 0))
        p = new LPCepstrum(p);
    else
        p = new Cepstrum(p);
    p = normaliseMean(p);
    p = deltas(p);
    p = normaliseVariance(p);
    return p;
}
#endif
