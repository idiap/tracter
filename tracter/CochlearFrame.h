/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef COCHLEARFRAME_H
#define COCHLEARFRAME_H

#include "CachedComponent.h"

namespace Tracter
{
    class CochlearFrame : public CachedComponent<float>
    {
    public:
        CochlearFrame(Component<float>* iInput,
                      const char* iObjectName = "CochlearFrame");

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        int mSize;
    };
}

#endif /* COCHLEARFRAME_H */
