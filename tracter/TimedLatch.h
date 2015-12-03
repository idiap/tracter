/*
 * Copyright 2011 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2011
 */

#ifndef TIMEDLATCH_H
#define TIMEDLATCH_H

#include <algorithm>

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Timed latch
     *
     * Latches to true or false if the input stays in that state for
     * long enough.
     */
    class TimedLatch : public CachedComponent<BoolType>
    {
    public:
        TimedLatch(Component<BoolType>* iInput,
                   const char* iObjectName = "TimedLatch");

    protected:
        bool UnaryFetch(IndexType iIndex, BoolType* oData);
        void Reset(bool iPropagate);

    private:
        Component<BoolType>* mInput;

        IndexType mIndex;
        bool mState;
        int mConfirmTrueTime;
        int mConfirmFalseTime;
    };
}

#endif /* TIMEDLATCH_H */
