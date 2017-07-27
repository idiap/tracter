/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MODULATION_H
#define MODULATION_H

#include <complex>

#include "CachedComponent.h"

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
        void reset()
        {
            mState = complex(0.0f, 0.0f);
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
    class Modulation : public CachedComponent<float>
    {
    public:
        Modulation(Component<float>* iInput,
                   const char* iObjectName = "Modulation");

    protected:

        bool unaryFetch(IndexType iIndex, float* oData);
        virtual void reset(bool iPropagate);

        void dotHook()
        {
            CachedComponent<float>::dotHook();
            dotRecord(1, "nBins=%d", mNBins);
            dotRecord(1, "ahead=%d", mLookAhead);
            dotRecord(1, "behind=%d", mLookBehind);
        }

    private:
        Component<float>* mInput;
        IndexType mIndex;
        int mNBins;
        SizeType mLookAhead;
        SizeType mLookBehind;
        SlidingDFT mDFT;
    };

}

#endif /* MODULATION_H */
