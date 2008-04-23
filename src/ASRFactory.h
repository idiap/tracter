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

namespace TTracter
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
    class ASRFactory : public TTracter::Object
    {
    public:
        ASRFactory(const char* iObjectName = "ASRFactory");
        Plugin<float>* Frontend(Plugin<float>* iPlugin);

    private:
        std::map<std::string, frontend_t> mFrontend;
        Plugin<float>* BasicFrontend(Plugin<float>* iPlugin);
    };
}

#endif /* ASRFACTORY_H */
