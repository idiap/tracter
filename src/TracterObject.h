/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef TRACTEROBJECT_H
#define TRACTEROBJECT_H

#include <exception>

/**
 * Tracter namespace
 */
namespace Tracter
{
    extern bool sInitialised;
    extern bool sShowConfig;
    extern int sVerbose;

    /**
     * Root of all tracter objects.
     *
     * This class defines basic functionality for tracter objects.  Right
     * now, this is just exceptions and the environment variable parameter
     * mechanism.
     *
     * A tracter object is an object with a name.  The name allows
     * parameters for the object to be picked up from an environment
     * variable prefixed with that name.  It also allows debugging to
     * identify which object gave rise to which behaviour.
     *
     * Tracter::Object also defines two global options: Verbose is a
     * numerical value corresponding to a verbosity level.  ShowConfig is
     * a boolean defining whether to output the configuration (parameters)
     * as it is consulted.
     */
    class Object
    {
    public:
        Object();
        virtual ~Object() throw () {} // Stops destructors throwing exceptions
        const char* ObjectName() const { return mObjectName; }

    protected:
        const char* mObjectName; ///< Name of this object

        float GetEnv(const char* iSuffix, float iDefault);
        int GetEnv(const char* iSuffix, int iDefault);
        const char* GetEnv(const char* iSuffix, const char* iDefault);

        void Verbose(int iLevel, const char* iString, ...);

    private:
        const char* getEnv(const char* iSuffix, const char* iDefault);
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

#endif /* TRACTEROBJECT_H */
