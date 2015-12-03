/*
 * Copyright 2010 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef LOG_H
#define LOG_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Calculate logarithm
     */
    class Log : public CachedComponent<float>
    {
    public:
        Log(
            Component<float>* iInput, const char* iObjectName = "Log"
        );
        virtual ~Log() throw();

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

        void DotHook()
        {
            CachedComponent<float>::DotHook();
            DotRecord(1, "floor=%.1e", mFloor);
            DotRecord(1, "log(floor)=%.1f", mLogFloor);
        }

    private:
        Component<float>* mInput;
        float mFloor;
        float mLogFloor;
        int mFloored;
    };
}

#endif /* LOG_H */
