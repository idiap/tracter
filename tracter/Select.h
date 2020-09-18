/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SELECT_H
#define SELECT_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Selects a sub-array
     */
    class Select : public CachedComponent<float>
    {
    public:
        Select(Component<float>* iInput, const char* iObjectName = "Select");

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

        void dotHook()
        {
            CachedComponent<float>::dotHook();
            dotRecord(1, "lo=%d", mLoIndex);
            dotRecord(1, "hi=%d", mHiIndex);
        }

    private:
        Component<float>* mInput;
        int mLoIndex;
        int mHiIndex;
    };
}

#endif /* SELECT_H */
