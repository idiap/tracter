/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef RESAMPLE_H
#define RESAMPLE_H

#include <vector>

#include "UnaryPlugin.h"

namespace Tracter
{
    /**
     * Resample or convert sample rate.
     * Currently tied to libresample
     */
    class Resample : public UnaryPlugin<float, float>
    {
    public:
        Resample(Plugin<float>* iInput,
                 const char* iObjectName = "Resample");
        virtual ~Resample() throw();
        void Reset(bool iPropagate = true);
        void MinSize(int iSize, int iReadBack, int iReadAhead);

    protected:
        int Fetch(IndexType iIndex, CacheArea& iOutputArea);

    private:
        void *mHandle;
        double mRatio;
        std::vector<float> mResample;
    };
}

#endif /* RESAMPLE_H */
