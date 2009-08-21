/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef ENERGY_H
#define ENERGY_H

#include "CachedComponent.h"

namespace Tracter
{
    class Energy : public CachedComponent<float>
    {
    public:
        Energy(Component<float>* iInput,
               const char* iObjectName = "Energy");

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
    };
}

#endif /* ENERGY_H */
