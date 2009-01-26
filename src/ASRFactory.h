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

namespace Tracter
{
    // Pre-declaration for the typedef
    class ASRFactory;
    typedef Plugin<float>* (ASRFactory::* frontend_t)(Plugin<float>*);
    typedef Plugin<float>* (ASRFactory::* source_t)(Source*&);

    /**
     * Factory class for Automatic Speech Recognition.
     *
     * Allows predefined graphs to be instantiated based on run-time
     * configuration variables.  The same configuration can (should!)
     * be used by both the feature extraction tool and the recogniser.
     *
     * Distingushes between source and front-end.  To get just a
     * source, specify the "Null" front-end.
     */
    class ASRFactory : public Tracter::Object
    {
    public:
        ASRFactory(const char* iObjectName = "ASRFactory");
        virtual ~ASRFactory() throw () {}
        Plugin<float>* CreateFrontend(Plugin<float>* iPlugin);
        Plugin<float>* CreateSource(Source*& iSource);

        Plugin<float>* GetSpeakerIDSource();

    protected:
        std::map<std::string, source_t> mSource;
        std::map<std::string, frontend_t> mFrontend;

        Plugin<float>* fileSource(Source*& iSource);
        Plugin<float>* sndFileSource(Source*& iSource);
        Plugin<float>* alsaSource(Source*& iSource);
        Plugin<float>* socketSource(Source*& iSource);
        Plugin<float>* htkSource(Source*& iSource);

        Plugin<float>* deltas(Plugin<float>* iPlugin);
        Plugin<float>* normaliseMean(Plugin<float>* iPlugin);
        Plugin<float>* normaliseVariance(Plugin<float>* iPlugin);

        Plugin<float>* nullFrontend(Plugin<float>* iPlugin);
        Plugin<float>* basicFrontend(Plugin<float>* iPlugin);
        Plugin<float>* basicVADFrontend(Plugin<float>* iPlugin);
        Plugin<float>* basicMLPVADFrontend(Plugin<float>* iPlugin);
        Plugin<float>* mlpvadFrontend(Plugin<float>* iPlugin);
        Plugin<float>* plpFrontend(Plugin<float>* iPlugin);
        Plugin<float>* htkFrontend(Plugin<float>* iPlugin);
        Plugin<float>* plpPosteriorFrontend(Plugin<float>* iPlugin);

    private:
        SpeakerIDSocketSource* mSpeakerIDSource;
    };

}

#endif /* ASRFACTORY_H */
