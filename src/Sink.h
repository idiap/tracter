/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SINK_H
#define SINK_H

#include <assert.h>

#include "PluginObject.h"

/**
 * A Sink is a (not necessarily the) final vertex of a directed graph;
 * one that has no arcs leading out of it.  In Tracter, a sink is a
 * plugin with no cache.  In this sense it is also typeless.
 *
 * It makes sense for a sink to form the base of something that
 * naturally pulls data, or for a sink to provide a method that pulls
 * data for non-tracter-aware routines.
 *
 * N.B., there is no destructor in this base class.  In particular, it
 * does not call Delete() as that function requires inputs to be
 * defined.  Classes derived from this, however, can choose to
 * implement a destructor that calls Delete().
 */
class Sink : public PluginObject
{
public:
    Sink()
    {
        mMinReadBack = 0;
        mMaxReadBack = 0;
        mMinReadAhead = 0;
        mMaxReadAhead = 0;
    }

    /** Fetch() should not be called on a sink */
    int Fetch(IndexType iIndex, CacheArea& iOutputArea)
    {
        assert(iIndex >= 0);
        assert(0);
    }

    /** Resize() should not be called on a sink */
    void Resize(int iSize)
    {
        assert(iSize >= 0);
        assert(0);
    }
};

#endif /* SINK_H */
