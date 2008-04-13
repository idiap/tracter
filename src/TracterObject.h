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
namespace TTracter {
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
namespace TTracter
{
    class Object
    {
    protected:
        const char* mObjectName;

        void test();
    };
}

void TTracter::Object::test()
{
    assert(0);
};

#endif /* TRACTEROBJECT_H */
