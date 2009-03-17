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

namespace Tracter
{
    /**
     * Type independent interface for a source plugin.
     */
    class ISource
    {
    public:
        virtual ~ISource() {}

        /** Open a source with the given name */
        virtual void Open(const char* iName) = 0;
        virtual void SetTime(TimeType iTime) = 0;

    protected:
        virtual TimeType TimeStamp(IndexType iIndex) = 0;
    };

    /**
     * Type dependent Source implementation
     *
     * The template type should be a typed plugin type.
     */
    template <class T>
    class Source : public ISource, public T
    {
        // Template type is not used after here, so maybe it should be
        // moved to a type independent class.
    public:
        Source() { mTime = 0; }
        virtual ~Source() throw () {}

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
                time += PluginObject::TimeOffset(iIndex);
            return time;
        }

        TimeType mTime;
    };
}

#endif /* SOURCE_H */
