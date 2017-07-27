/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef DIVIDE_H
#define DIVIDE_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Divides a first input by a second.
     */
    class Divide : public CachedComponent<float>
    {
    public:
        Divide(Component<float>* iInput1, Component<float>* iInput2,
               const char* iObjectName = "Divide");

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput1;
        Component<float>* mInput2;
    };
}

#endif /* DIVIDE_H */
