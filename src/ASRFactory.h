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

namespace Tracter
{
    // Pre-declaration for the typedef
    class ASRFactory;
    typedef Plugin<float>* (ASRFactory::* frontend_t)(Plugin<float>*);

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
        Plugin<float>* Frontend(Plugin<float>* iPlugin);

    protected:
        std::map<std::string, frontend_t> mFrontend;

    private:
        Plugin<float>* BasicFrontend(Plugin<float>* iPlugin);
        Plugin<float>* NoiseFrontend(Plugin<float>* iPlugin);
        Plugin<float>* PLPFrontend(Plugin<float>* iPlugin);
        Plugin<float>* ComplexFrontend(Plugin<float>* iPlugin);
    };
}

#endif /* ASRFACTORY_H */
