/*
 * Copyright 2016 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, January 2016
 */

#ifndef COCHLEARFILTER_H
#define COCHLEARFILTER_H

#include "CachedComponent.h"
#include <ssp/cochlea.h>

namespace Tracter
{
    /**
     * Implements a gamma-tone filter-bank
     */
    class CochlearFilter : public CachedComponent<float>
    {
    public:
        CochlearFilter(
            Component<float>* iInput, const char* iObjectName = "CochlearFilter"
        );
        virtual ~CochlearFilter() throw();
        void minSize(SizeType iSize, SizeType iReadBehind, SizeType iReadAhead);

    protected:
        SizeType contiguousFetch(
            IndexType iIndex, SizeType iLength, SizeType iOffset
        );
        virtual void reset(bool iPropagate);

    private:
        Component<float>* mInput;
        IndexType mIndex;
        ssp::Cochlea* mCochlea;
    };
}

#endif /* COCHLEARFILTER_H */
