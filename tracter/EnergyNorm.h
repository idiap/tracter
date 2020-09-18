/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef ENERGY_NORM_H
#define ENERGY_NORM_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Subtracts a second input from a first.
     */
    class EnergyNorm : public CachedComponent<float>
    {
    public:
      EnergyNorm(Component<float>* iInput1, /*Component<float>* iInput2,*/
                 const char* iObjectName = "EnergyNorm");

    protected:
      bool unaryFetch(IndexType iIndex, float* oData);

    private:
      Component<float>* mInput1;
      //Component<float>* mInput2;

      float m_maxE;
    };
}

#endif /* ENERGY_NORM_H */
