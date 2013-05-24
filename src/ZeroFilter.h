/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef ZEROFILTER_H
#define ZEROFILTER_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Implements a filter consisting of a single zero.
     */
    class ZeroFilter : public CachedComponent<float>
    {
    public:
        ZeroFilter(
            Component<float>* iInput, const char* iObjectName = "ZeroFilter"
        );
        void MinSize(SizeType iSize, SizeType iReadBehind, SizeType iReadAhead);

    protected:
        SizeType Fetch(IndexType iIndex, CacheArea& iOutputArea);

    private:
        Component<float>* mInput;
        float mZero;
    };
}

#endif /* ZEROFILTER_H */
