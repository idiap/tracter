/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOURCE_H
#define SOURCE_H

#include <cassert>

#include "PluginObject.h" // For TimeType

/**
 * Interface for a source plugin.  As a source plugin has basically
 * the same interface as any other plugin, this is designed to be
 * multiply inherited.  i.e., a source plugin should inherit this
 * *and* a typed plugin.
 */
namespace Tracter
{
    class Source
    {
    public:
        Source() { mTime = 0; }

        /** Open a source with the given name */
        virtual void Open(const char* iName) = 0;
        virtual ~Source() {}
        virtual void SetTime(TimeType iTime)
        {
            assert(iTime >= 0);
            mTime = iTime;
        }

    protected:

        virtual TimeType TimeStamp(IndexType iIndex)
        {
            TimeType time = mTime;
            if (iIndex)
                time += ((PluginObject*)this)->TimeOffset(iIndex);
            return time;
        }

        TimeType mTime;
    };
}

#endif /* SOURCE_H */
