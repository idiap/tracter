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
        virtual ~Log();

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

        void dotHook()
        {
            CachedComponent<float>::dotHook();
            dotRecord(1, "floor=%.1e", mFloor);
            dotRecord(1, "log(floor)=%.1f", mLogFloor);
        }

    private:
        Component<float>* mInput;
        float mFloor;
        float mLogFloor;
        int mFloored;
    };
}

#endif /* LOG_H */
