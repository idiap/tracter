/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SPECTRALSUBTRACT_H
#define SPECTRALSUBTRACT_H

#include "CachedPlugin.h"

namespace Tracter
{
    /**
     * Subtracts a second input from a first with flooring.
     */
    class SpectralSubtract : public CachedPlugin<float>
    {
    public:
        SpectralSubtract(Plugin<float>* iInput1, Plugin<float>* iInput2,
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
        PluginObject* GetInput(int iInput);
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        Plugin<float>* mInput1;
        Plugin<float>* mInput2;
        float mAlpha;
        float mBeta;
    };
}

#endif /* SPECTRALSUBTRACT_H */
