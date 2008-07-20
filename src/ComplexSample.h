/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef COMPLEXSAMPLE_H
#define COMPLEXSAMPLE_H

#include <complex>

#include "UnaryPlugin.h"

namespace Tracter
{
    typedef std::complex<float> complex;

    /**
     * Implements a sample consisting of a single complex.
     */
    class ComplexSample : public UnaryPlugin<complex, float>
    {
    public:
        ComplexSample(Plugin<float>* iInput,
                      const char* iObjectName = "ComplexSample");
        void MinSize(int iSize, int iReadAhead);

    protected:
        int Fetch(IndexType iIndex, CacheArea& iOutputArea);
    };
}

#endif /* COMPLEXSAMPLE_H */
