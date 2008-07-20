/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
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
            Plugin<float>* iInput, const char* iObjectName = "GeometricNoise"
        )
            :Noise(iInput, iObjectName)
        {
        }


    protected:

        /**
         * Accumulator.  Overridden for geometric mean
         */
        virtual float Accumulate(float iAccumulator, float iInput)
        {
            float ret = iAccumulator + logf(iInput);
            return ret;
        }

        /**
         * Mean Calculator.  Overridden for geometric mean
         */
        virtual float Calculate(float iAccumulator, int iN)
        {
            float ret = expf(iAccumulator / iN);
            return ret;
        }
    };
}

#endif /* GEOMETRICNOISE_H */
