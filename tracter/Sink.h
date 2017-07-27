/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SINK_H
#define SINK_H

#include <cassert>

#include "Component.h"

namespace Tracter
{
    /**
     * A Sink is a (not necessarily the) final vertex of a directed graph;
     * one that has no arcs leading out of it.  In Tracter, a sink is a
     * component with no cache.  In this sense it is also typeless.
     *
     * It makes sense for a sink to form the base of something that
     * naturally pulls data, or for a sink to provide a method that pulls
     * data for non-tracter-aware routines.
     */
    class Sink : public ComponentBase
    {
        /*
         * PNG: As of September 2010, I'm not sure what this comment
         * is on about.  I put the Delete here because it's easy to
         * forget otherwise.
         *
         * " N.B., there is no destructor in this base class.  In
         * particular, it does not call destruct() as that function
         * requires inputs to be defined.  Classes derived from this,
         * however, can choose to implement a destructor that calls
         * destruct(). "
         */
    public:
        Sink()
        {
            mMinReadBehind = 0;
            mMaxReadBehind = 0;
            mMinReadAhead = 0;
            mMaxReadAhead = 0;
        }
        virtual ~Sink() throw () { destruct(); }

        /** fetch() should not be called on a sink */
        SizeType fetch(IndexType iIndex, CacheArea& iOutputArea)
        {
            assert(iIndex >= 0);
            assert(0);
        }

        /** resize() should not be called on a sink */
        void resize(SizeType iSize)
        {
            assert(iSize >= 0);
            assert(0);
        }
    };
}

#endif /* SINK_H */
