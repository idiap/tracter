/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef NOISE_H
#define NOISE_H

#include "UnaryPlugin.h"

namespace Tracter
{
    /**
     * Stores a noise estimate
     *
     * Reads the first NInit (default 10) frames and uses them to form a
     * noise estimate.  Typically, the input is a Periodogram.
     */
    class Noise : public UnaryPlugin<float, float>
    {
    public:
        Noise(Plugin<float>* iInput, const char* iObjectName = "Noise");
        virtual void Reset(bool iPropagate);

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

        /**
         * Really basic accumulator.  However, it can be overridden by a
         * derived class to implement accumulation of a function of the
         * input.
         */
        virtual float Accumulate(float iAccumulator, float iInput)
        {
            float ret = iAccumulator + iInput;
            return ret;
        }

        /**
         * Calculate the mean of accumulated samples.  Just a division,
         * but can be overridden by a derived class to implement, say, a
         * geometric mean.
         */
        virtual float Calculate(float iAccumulator, int iN)
        {
            float ret = iAccumulator / iN;
            return ret;
        }

    private:
        bool mValid;
        bool mEnd;
        int mNInit;
    };
}

#endif /* NOISE_H */
