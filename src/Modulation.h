/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MODULATION_H
#define MODULATION_H

#include <complex>

#include "UnaryPlugin.h"

namespace Tracter
{
    typedef std::complex<float> complex;

    /**
     * Sliding DFT
     *
     * An efficient way of calculating a single bin of a DFT.
     */
    class SlidingDFT
    {
    public:
        void SetRotation(int iBin, int iNBins);
        const complex& Transform(float iNew, float iOld);
        void Reset()
        {
            mState.real() = 0.0f;
            mState.imag() = 0.0f;
        }

    private:
        complex mState;
        complex mRotation;
    };

    /**
     * A feature based on modulation.
     *
     * The input is filtered with a sliding DFT giving a feature that
     * is similar in principle to RASTA.
     */
    class Modulation : public CachedPlugin<float>
    {
    public:
        Modulation(Plugin<float>* iInput,
                   const char* iObjectName = "Modulation");

    protected:

        PluginObject* GetInput(int iInput)
        {
            assert(iInput == 0);
            assert(this->mNInputs == 1);
            return mInput;
        }

        bool UnaryFetch(IndexType iIndex, int iOffset);
        virtual void Reset(bool iPropagate);

    private:
        Plugin<float>* mInput;
        IndexType mIndex;
        int mNBins;
        int mLookAhead;
        int mLookBehind;
        SlidingDFT mDFT;
    };

}

#endif /* MODULATION_H */
