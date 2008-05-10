/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef TRACTEROBJECT_H
#define TRACTEROBJECT_H

/**
 * Tracter namespace
 */
namespace Tracter {
    extern bool sShowConfig;
    extern int sVerbose;
}

/**
 * Root of all tracter objects.
 *
 * This class defines basic functionality for tracter objects.
 *
 * Right now, this is just the environment variable parameter
 * mechanism.
 */
namespace Tracter
{
    class Object
    {
    protected:
        const char* mObjectName; ///< Name of this object

        float GetEnv(const char* iSuffix, float iDefault);
        int GetEnv(const char* iSuffix, int iDefault);
        const char* GetEnv(const char* iSuffix, const char* iDefault);

    private:
        const char* getEnv(const char* iSuffix, const char* iDefault);
    };
}

#endif /* TRACTEROBJECT_H */
