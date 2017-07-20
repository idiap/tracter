/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <exception>

#include <lube.h>
#include <lube/config.h>

/**
 * Tracter namespace
 */
namespace Tracter
{
    extern int sVerbose;

    /** String to enumerated value mapping */
    struct StringEnum
    {
        const char* str;
        int val;
    };


    /**
     * Root of all tracter objects.
     *
     * This class defines basic functionality for tracter objects.  Right now,
     * this is just exceptions and the configuration parameter mechanism.
     *
     * A tracter object is an object with a name.  The name allows parameters
     * for the object to be picked up from an configuration prefixed with that
     * name.  It also allows debugging to identify which object gave rise to
     * which behaviour.
     *
     * Tracter::Object also defines a option: Verbose is a numerical
     * value corresponding to a verbosity level.
     */
    class Object : public lube::Config
    {
    public:
        //Object();
        virtual ~Object() throw () {} // Stops destructors throwing exceptions
        const char* objectName() { return configStr(); }
        void objectName(const char* iName) { configSection(iName); }

    protected:
        const char* mObjectName; ///< Name of this object

        float GetEnv(const char* iSuffix, float iDefault);
        int GetEnv(const char* iSuffix, int iDefault);
        const char* GetEnv(const char* iSuffix, const char* iDefault);
        int GetEnv(const StringEnum* iStringEnum, int iDefault);

        void verbose(var iVerbose);
        void Verbose(int iLevel, const char* iString, ...) const;

    private:
        const char* getEnv(
            const char* iSuffix, const char* iDefault, bool iEcho = true
        );
    };

    const int STRING_SIZE = 512;
    class Exception: public std::exception
    {
    public:
        Exception(const char* iString, ...);
        virtual const char* what() const throw()
        {
            return mString;
        }
    private:
        char mString[STRING_SIZE];
    };
}

#endif /* OBJECT_H */
