/*
 * Copyright 2010 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef TRANSVERSEFILTER_H
#define TRANSVERSEFILTER_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Run a filter along the frame.
     */
    class TransverseFilter : public CachedComponent<float>
    {
    public:
        TransverseFilter(
            Component<float>* iInput, const
            char* iObjectName = "TransverseFilter"
        );

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        int mRadius;
    };
}

#endif /* TRANSVERSEFILTER_H */
