/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef NOISE_H
#define NOISE_H

#include <vector>

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Stores a noise estimate
     *
     * Reads the first NInit (default 10) frames and uses them to form a
     * noise estimate.  Typically, the input is a Periodogram.
     */
    class Noise : public CachedComponent<float>
    {
    public:
        Noise(Component<float>* iInput, const char* iObjectName = "Noise");
        virtual ~Noise() throw ();
        virtual void Reset(bool iPropagate);

    protected:
        std::vector<float> mAccumulator;
        int mNAccumulated;
        bool UnaryFetch(IndexType iIndex, float* oData);

        /**
         * Really basic accumulator.  However, it can be overridden by a
         * derived class to implement accumulation of a function of the
         * input.
         */
        virtual void Accumulate(float* iInput)
        {
            assert(mFrame.size);
            assert(iInput);
            for (int i=0; i<mFrame.size; i++)
                mAccumulator[i] += iInput[i];
            mNAccumulated++;
        }

        /**
         * Calculate a noise estimate by dividing the accumulator by
         * the number of samples.  Can be overridden by a derived
         * class to implement geometric mean or similar.
         */
        virtual void Calculate(float* oOutput)
        {
            assert(mFrame.size);
            assert(oOutput);
            for (int i=0; i<mFrame.size; i++)
                oOutput[i] = mAccumulator[i] / mNAccumulated;
        }

    private:
        Component<float>* mInput;
        bool mValid;
        bool mEnd;
        int mNInit;
        bool mSoftReset;
        bool mWrite;
    };
}

#endif /* NOISE_H */
