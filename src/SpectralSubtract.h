/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SPECTRALSUBTRACT_H
#define SPECTRALSUBTRACT_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Subtracts a second input from a first with flooring.
     */
    class SpectralSubtract : public CachedComponent<float>
    {
    public:
        SpectralSubtract(Component<float>* iInput1, Component<float>* iInput2,
                         const char* iObjectName = "SpectralSubtract");

        /** Set the oversubtraction factor */
        void SetAlpha(float iAlpha)
        {
            assert(iAlpha > 0.0f);
            mAlpha = iAlpha;
        }

        /** Set the flooring factor */
        void SetBeta(float iBeta)
        {
            assert(iBeta > 0.0f);
            mBeta = iBeta;
        }

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput1;
        Component<float>* mInput2;
        float mAlpha;
        float mBeta;
    };
}

#endif /* SPECTRALSUBTRACT_H */
