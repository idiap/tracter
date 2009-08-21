/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef COMPLEXSAMPLE_H
#define COMPLEXSAMPLE_H

#include <complex>

#include "CachedComponent.h"

namespace Tracter
{
    typedef std::complex<float> complex;

    /**
     * Implements a sample consisting of a single complex.
     */
    class ComplexSample : public CachedComponent<complex>
    {
    public:
        ComplexSample(Component<float>* iInput,
                      const char* iObjectName = "ComplexSample");
        void MinSize(int iSize, int iReadBehind, int iReadAhead);

    protected:
        int Fetch(IndexType iIndex, CacheArea& iOutputArea);

    private:
        Component<float>* mInput;
    };
}

#endif /* COMPLEXSAMPLE_H */
