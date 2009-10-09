/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * Copyright 2008 by The University of Sheffield
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef HTKLIB_H
#define HTKLIB_H

#include "TracterObject.h"

namespace Tracter
{
    class HTKLibSource;

    /**
     * HTK library manager class.  This always exists if there is a
     * possiblility of a program using HTK.  When HTK is not actually
     * compiled in, it returns null pointers and the like.  When HTK
     * is present, it contains pointers to aspects of HTK that are
     * naturally singleton.
     */
    class HTKLib : Object
    {
        friend class HTKLibSource;

    public:
        HTKLib();
        void Initialise(const char* iScript);

        HTKLibSource* mHTKLibSource;

    private:

        void InitialiseHTK(const char* iConfig, const char* iScript);
    };

    /** The lone static instantiation of HTKLib */
    extern HTKLib sHTKLib;
}

#endif /* HTKLIB_H */
