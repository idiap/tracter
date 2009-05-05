/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef ENERGY_H
#define ENERGY_H

#include "UnaryPlugin.h"

namespace Tracter
{
    class Energy : public UnaryPlugin<float, float>
    {
    public:
        Energy(Plugin<float>* iInput,
               const char* iObjectName = "Energy");

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);
    };
}

#endif /* ENERGY_H */
