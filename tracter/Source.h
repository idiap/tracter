/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOURCE_H
#define SOURCE_H

#include <cassert>

#include "Component.h" // For TimeType

namespace Tracter
{
    /**
     * Type independent interface for a source component.
     */
    class ISource
    {
    public:
        virtual ~ISource() {}

        /** Open a source with the given name */
        virtual void open(
            const char* iName,        ///< Name of file or device
            TimeType iBeginTime = -1, ///< Time for 0'th frame
            TimeType iEndTime = -1    ///< Time for last frame
        ) = 0;

        /** Associate a time with index 0 of the source */
        virtual void setTime(TimeType iTime) = 0;

    protected:
        virtual TimeType timeStamp(IndexType iIndex = 0) const = 0;
    };

    /**
     * Type dependent Source implementation
     *
     * The template type should be a typed component type.
     */
    template <class T>
    class Source : public ISource, public T
    {
        // Template type is not used after here, so maybe it should be
        // moved to a type independent class.
    public:
        Source() { mTime = 0; mFrameRate = 0.0f; }

        virtual void setTime(TimeType iTime)
        {
            assert(iTime >= 0);
            mTime = iTime;
        }

    protected:

        virtual TimeType timeStamp(IndexType iIndex) const
        {
            TimeType time = mTime;
            if (iIndex)
                time += ComponentBase::timeOffset(iIndex);
            return time;
        }

        virtual ExactRateType exactFrameRate() const
        {
            ExactRateType r;
            r.rate = mFrameRate;
            r.period = 1.0f;
            return r;
        }

        TimeType mTime;
        float mFrameRate;
    };
}

#endif /* SOURCE_H */
