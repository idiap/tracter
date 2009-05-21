/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PERIODOGRAM_H
#define PERIODOGRAM_H

#include "Window.h"
#include "Fourier.h"
#include "UnaryPlugin.h"

namespace Tracter
{
    /**
     * Calculate a periodogram (aka power spectral density).  Uses the
     * FFTW library, which is not tied to DFT sizes of powers of 2,
     * but does have a significant startup time.  A window is
     * included; the windowing is beneficial in that it is done during
     * a copy from cache memory to aligned memory.
     */
    class Periodogram : public UnaryPlugin<float, float>
    {
    public:
        Periodogram(Plugin<float>* iInput,
                    const char* iObjectName = "Periodogram");
        virtual ~Periodogram() throw();

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        float* mRealData;
        complex* mComplexData;
        Window* mWindow;
        Fourier mFourier;
    };
}

#endif /* PERIODOGRAM_H */
