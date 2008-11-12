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

namespace Tracter
{
    // Pre-declaration for the typedef
    class ASRFactory;
    typedef Plugin<float>* (ASRFactory::* frontend_t)(Plugin<float>*);
    typedef Plugin<float>* (ASRFactory::* source_t)(Source*&);

    /**
     * ASR Factory.  Allocates graphs of plugins to do things related
     * to Automatic Speech Recognition.  The factory is itself a
     * Tracter::Object, so it can use environment variables to
     * customise its behaviour.
     */
    class ASRFactory : public Tracter::Object
    {
    public:
        ASRFactory(const char* iObjectName = "ASRFactory");
        virtual ~ASRFactory() throw () {}
        Plugin<float>* CreateFrontend(Plugin<float>* iPlugin);
        Plugin<float>* CreateSource(Source*& iSource);

    protected:
        std::map<std::string, source_t> mSource;
        std::map<std::string, frontend_t> mFrontend;

        Plugin<float>* fileSource(Source*& iSource);
        Plugin<float>* alsaSource(Source*& iSource);
        Plugin<float>* socketSource(Source*& iSource);

        Plugin<float>* deltas(Plugin<float>* iPlugin);
        Plugin<float>* normaliseMean(Plugin<float>* iPlugin);
        Plugin<float>* normaliseVariance(Plugin<float>* iPlugin);

        Plugin<float>* basicFrontend(Plugin<float>* iPlugin);
        Plugin<float>* basicVADFrontend(Plugin<float>* iPlugin);
        Plugin<float>* plpFrontend(Plugin<float>* iPlugin);
        Plugin<float>* htkFrontend(Plugin<float>* iPlugin);
        Plugin<float>* posteriorFrontend(Plugin<float>* iPlugin);
    };
}

#endif /* ASRFACTORY_H */
