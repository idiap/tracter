/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef LPCEPSTRUM_H
#define LPCEPSTRUM_H

#include "Fourier.h"
#include "CachedComponent.h"

namespace Tracter
{
    /**
     * LPCepstrum analysis from a warped spectrum
     */
    class LPCepstrum : public CachedComponent<float>
    {
    public:
        LPCepstrum(
            Component<float>* iInput, const char* iObjectName = "LPCepstrum"
        );
        virtual ~LPCepstrum() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        int mOrder;
        int mNCompressed;
        int mNCepstra;
        bool mC0;
        float mCompressionPower;
        float* mCompressed;
        float* mAutoCorrelation;
        Fourier mFourier;
    };
}

#endif /* LPCEPSTRUM_H */
