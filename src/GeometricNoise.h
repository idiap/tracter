/*
 * Copyright 2008 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef GEOMETRICNOISE_H
#define GEOMETRICNOISE_H

#include <cmath>

#include "Noise.h"

namespace Tracter
{
    /**
     * Stores a noise estimate using geometric mean
     *
     * Functionality is identical to Noise, except that it implements a
     * geometric mean instead of arithmetic.
     */
    class GeometricNoise : public Noise
    {
    public:
        /**
         * Constructor.  Just sets the object name.
         */
        GeometricNoise(
            Component<float>* iInput, const char* iObjectName = "GeometricNoise"
        )
            :Noise(iInput, iObjectName)
        {
        }


    protected:

        /**
         * Accumulator.  Overridden for geometric mean
         */
        virtual void Accumulate(float* iInput)
        {
            assert(mFrame.size);
            assert(iInput);
            for (int i=0; i<mFrame.size; i++)
                mAccumulator[i] += logf(iInput[i]);
            mNAccumulated++;
        }

        /**
         * Mean Calculator.  Overridden for geometric mean
         */
        virtual void Calculate(float* oOutput)
        {
            assert(mFrame.size);
            assert(oOutput);
            for (int i=0; i<mFrame.size; i++)
                oOutput[i] = expf(mAccumulator[i] / mNAccumulated);
        }
    };
}

#endif /* GEOMETRICNOISE_H */
