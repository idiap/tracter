/*
 * Copyright 2010 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FOURIERTRANSFORM_H
#define FOURIERTRANSFORM_H

#include "Window.h"
#include "Fourier.h"
#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Complex Fourier Transform from real data.
     */
    class FourierTransformR2C : public CachedComponent<complex>
    {
    public:
        /**
         * The constructor includes an optional window; the windowing
         * is beneficial in that it is done during a copy from cache
         * memory to aligned memory.
         */
        FourierTransformR2C(Component<float>* iInput,
                         const char* iObjectName = "FourierTransform");
        virtual ~FourierTransformR2C();

    protected:
        bool unaryFetch(IndexType iIndex, complex* oData);

    private:
        Component<float>* mInput;
        float* mRealData;
        complex* mComplexData;
        Window* mWindow;
        Fourier mFourier;
    };

    /**
     * Real Fourier Transform from complex data.
     */
    class FourierTransformC2R : public CachedComponent<float>
    {
    public:
        /**
         * The constructor includes an optional window; the windowing
         * is beneficial in that it is done during a copy from cache
         * memory to aligned memory.
         */
        FourierTransformC2R(Component<complex>* iInput,
                         const char* iObjectName = "FourierTransform");
        virtual ~FourierTransformC2R();

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<complex>* mInput;
        float* mRealData;
        complex* mComplexData;
        Window* mWindow;
        Fourier mFourier;
    };
}

#endif /* FOURIERTRANSFORM_H */
