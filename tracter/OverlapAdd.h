/*
 * Copyright 2010 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef OVERLAPADD_H
#define OVERLAPADD_H

#include "CachedComponent.h"

namespace Tracter
{
    class OverlapAdd : public CachedComponent<float>
    {
    public:
        OverlapAdd(Component<float>* iInput,
                   const char* iObjectName = "OverlapAdd");

    protected:
        int ContiguousFetch(IndexType iIndex, int iLength, int iOffset);

    private:
        Component<float>* mInput;
    };
}


#endif /* OVERLAPADD_H */
