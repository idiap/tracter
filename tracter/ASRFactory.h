/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef ASRFACTORY_H
#define ASRFACTORY_H

#include <map>
#include <string>

#include "tracter/Object.h"
#include "Component.h"
#include "Source.h"

#define DECLARE_GRAPH_FACTORY(name)                             \
    class name##GraphFactory : public GraphFactory              \
    {                                                           \
    public:                                                     \
        name##GraphFactory(const char* iObjectName = #name)     \
        {                                                       \
            objectName(iObjectName);                            \
        }                                                       \
        Component<float>* create(Component<float>* iComponent); \
    };

#define DECLARE_SOURCE_FACTORY(name)                            \
    class name##SourceFactory : public SourceFactory            \
    {                                                           \
    public:                                                     \
        name##SourceFactory(const char* iObjectName = #name)    \
        {                                                       \
            objectName(iObjectName);                            \
        }                                                       \
        Component<float>* create(ISource*& iSource);            \
    };

namespace Tracter
{
    /**
     * Source factory.  Abstract class for factories generating
     * sources.
     */
    class SourceFactory : public Tracter::Object
    {
    public:
        virtual ~SourceFactory() {}
        virtual Component<float>* create(ISource*& iSource) = 0;
    };

    /**
     * Graph factory.  Abstract class for factories generating
     * graphs.
     */
    class GraphFactory : public Tracter::Object
    {
    public:
        virtual ~GraphFactory() {}
        virtual Component<float>* create(Component<float>* iComponent) = 0;

    protected:
        Component<float>* deltas(Component<float>* iComponent);
        Component<float>* normaliseMean(Component<float>* iComponent);
        Component<float>* normaliseVariance(Component<float>* iComponent);
    };

    DECLARE_SOURCE_FACTORY(File)
    DECLARE_SOURCE_FACTORY(SndFile)
    DECLARE_SOURCE_FACTORY(ALSA)
    DECLARE_SOURCE_FACTORY(StreamSocket)
    DECLARE_SOURCE_FACTORY(HTK)
    DECLARE_SOURCE_FACTORY(LNA)
    DECLARE_SOURCE_FACTORY(RtAudio)
    DECLARE_SOURCE_FACTORY(PulseAudio)

    DECLARE_GRAPH_FACTORY(Null)
    DECLARE_GRAPH_FACTORY(CMVN)
    DECLARE_GRAPH_FACTORY(Basic)
    DECLARE_GRAPH_FACTORY(BasicSpeechDet)
    DECLARE_GRAPH_FACTORY(BasicVAD)
    DECLARE_GRAPH_FACTORY(PLP)
    DECLARE_GRAPH_FACTORY(PLPVAD)
    DECLARE_GRAPH_FACTORY(MCep)
    DECLARE_GRAPH_FACTORY(Cochlear)
    DECLARE_GRAPH_FACTORY(CochlearSNR)
    DECLARE_GRAPH_FACTORY(SNR)

    /**
     * Factory class for Automatic Speech Recognition.
     *
     * Allows predefined graphs to be instantiated based on run-time
     * configuration variables.  The same configuration can (should!)
     * be used by both the feature extraction tool and the recogniser.
     *
     * Distingushes between source and front-end.  To get just a
     * source, specify the "Null" front-end.
     *
     * This broadly follows the design pattern where factory methods
     * evolve into more complicated structures.  Originally it was a
     * bunch of factory methods.  Now it's more of a builder
     * enveloping several factories, if that is a valid pattern.
     */
    class ASRFactory : public Tracter::Object
    {
    public:
        ASRFactory(const char* iObjectName = "ASRFactory");
        virtual ~ASRFactory();
        Component<float>* createFrontend(Component<float>* iComponent);
        Component<float>* createSource(ISource*& iSource);

        /** Register a source factory in the builder */
        void registerSource(SourceFactory* iSource)
        {
            mSource[iSource->objectName()] = iSource;
        }

        /** Register a graph factory for front-ends in the builder */
        void registerFrontend(GraphFactory* iFrontend)
        {
            mFrontend[iFrontend->objectName()] = iFrontend;
        }

    protected:
        std::map<std::string, SourceFactory*> mSource;
        std::map<std::string, GraphFactory*> mFrontend;
    };

}

#endif /* ASRFACTORY_H */
