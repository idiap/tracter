/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef RESAMPLE_H
#define RESAMPLE_H

#include "CachedComponent.h"

#ifdef HAVE_LIBRESAMPLE
# define HAVE_RESAMPLE = 1
#endif

namespace Tracter
{
    /**
     * Implementation specific data
     */
    struct ResampleData;

    /**
     * Resample or convert sample rate.
     * Currently tied to libresample
     */
    class Resample : public CachedComponent<float>
    {
    public:
        Resample(Component<float>* iInput,
                 const char* iObjectName = "Resample");
        virtual ~Resample() throw();
        void Reset(bool iPropagate = true);
        void MinSize(int iSize, int iReadBehind, int iReadAhead);

    protected:
        int Fetch(IndexType iIndex, CacheArea& iOutputArea);

    private:
        Component<float>* mInput;
        ResampleData *mResampleData;
    };
}

#endif /* RESAMPLE_H */
