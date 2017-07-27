/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SUBTRACT_H
#define SUBTRACT_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Subtracts a second input from a first.
     */
    class Subtract : public CachedComponent<float>
    {
    public:
        Subtract(Component<float>* iInput1, Component<float>* iInput2,
                 const char* iObjectName = "Subtract");

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput1;
        Component<float>* mInput2;
    };
}

#endif /* SUBTRACT_H */
