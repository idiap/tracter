/*
 * Copyright 2011 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2011
 */

#ifndef COMPARATOR_H
#define COMPARATOR_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * A comparator, ostensibly for building a VAD
     */
    class Comparator : public CachedComponent<BoolType>
    {
    public:
        Comparator(Component<float>* iInput1,
                   Component<float>* iInput2,
                   const char* iObjectName = "Comparator");

    protected:

        bool unaryFetch(IndexType iIndex, BoolType* oData);

        void dotHook()
        {
            CachedComponent<BoolType>::dotHook();
            dotRecord(1, "thres=%.2fdB", 10.0f * log10f(mThreshold));
        }

    private:

        Component<float>* mInput1;
        Component<float>* mInput2;
        float mThreshold;
        bool mShowGuts;
    };
}

#endif /* COMPARATOR_H */
