/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef CEPSTRUM_H
#define CEPSTRUM_H

#include "Fourier.h"
#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Generate the cepstrum of an input
     */
    class Cepstrum : public CachedComponent<float>
    {
    public:
        Cepstrum(Component<float>* iInput, const char* iObjectName = "Cepstrum");
        virtual ~Cepstrum();

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        float mFloor;
        float mLogFloor;
        int mFloored;
        int mNLogData;
        int mNCepstra;
        float* mLogData;
        float* mCepstra;
        bool mC0;
        Fourier mFourier;
    };
}

#endif /* CEPSTRUM_H */
