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

#include "TracterObject.h"
#include "Plugin.h"
#include "Source.h"
#include "SpeakerIDSocketSource.h"

#define DECLARE_GRAPH_FACTORY(name) \
    class name##GraphFactory : public GraphFactory \
    { \
    public: \
        name##GraphFactory(const char* iObjectName = #name) \
        { \
            mObjectName = iObjectName; \
        } \
        Plugin<float>* Create(Plugin<float>* iPlugin); \
    };

#define DECLARE_SOURCE_FACTORY(name) \
    class name##SourceFactory : public SourceFactory \
    { \
    public: \
        name##SourceFactory(const char* iObjectName = #name) \
        { \
            mObjectName = iObjectName; \
        } \
        Plugin<float>* Create(ISource*& iSource); \
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
        virtual ~SourceFactory() throw () {}
        virtual Plugin<float>* Create(ISource*& iSource) = 0;
    };

    /**
     * Graph factory.  Abstract class for factories generating
     * graphs.
     */
    class GraphFactory : public Tracter::Object
    {
    public:
        virtual ~GraphFactory() throw () {}
        virtual Plugin<float>* Create(Plugin<float>* iPlugin) = 0;

    protected:
        Plugin<float>* deltas(Plugin<float>* iPlugin);
        Plugin<float>* normaliseMean(Plugin<float>* iPlugin);
        Plugin<float>* normaliseVariance(Plugin<float>* iPlugin);
    };

    DECLARE_SOURCE_FACTORY(File)
    DECLARE_SOURCE_FACTORY(SndFile)
    DECLARE_SOURCE_FACTORY(ALSA)
    DECLARE_SOURCE_FACTORY(StreamSocket)
    DECLARE_SOURCE_FACTORY(HTKLib)
    DECLARE_SOURCE_FACTORY(HTK)

    DECLARE_GRAPH_FACTORY(Null)
    DECLARE_GRAPH_FACTORY(Basic)
    DECLARE_GRAPH_FACTORY(BasicVAD)
    DECLARE_GRAPH_FACTORY(BasicMLPVAD)
    DECLARE_GRAPH_FACTORY(MLPVAD)
    DECLARE_GRAPH_FACTORY(PLP)
    DECLARE_GRAPH_FACTORY(HTK)
    DECLARE_GRAPH_FACTORY(PLPPosterior)
    DECLARE_GRAPH_FACTORY(MCep)

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
        virtual ~ASRFactory() throw ();
        Plugin<float>* CreateFrontend(Plugin<float>* iPlugin);
        Plugin<float>* CreateSource(ISource*& iSource);

        /** Register a source factory in the builder */
        void RegisterSource(SourceFactory* iSource)
        {
            mSource[iSource->ObjectName()] = iSource;
        }

        /** Register a graph factory for front-ends in the builder */
        void RegisterFrontend(GraphFactory* iFrontend)
        {
            mFrontend[iFrontend->ObjectName()] = iFrontend;
        }

        Plugin<float>* GetSpeakerIDSource();

    protected:
        std::map<std::string, SourceFactory*> mSource;
        std::map<std::string, GraphFactory*> mFrontend;

    private:
        SpeakerIDSocketSource* mSpeakerIDSource;
    };

}

#endif /* ASRFACTORY_H */
