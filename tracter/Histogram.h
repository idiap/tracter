/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Writes a Histogram representing the input
     */
    class Histogram : public CachedComponent<float>
    {
    public:
        Histogram(
            Component<float>* iInput, const char* iObjectName = "Histogram"
        );
        virtual ~Histogram();

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        void writeMode();
        void writeHistogram();

        Component<float>* mInput;
        float mMin;
        float mMax;
        float mScale;
        int mCount;
        int mMinCount;
        int mNBins;
        std::vector< std::vector<float> > mBin;
        bool mPDF;
        float mPower;
        bool mUnPower;
        bool mMode;
    };
}

#endif /* HISTOGRAM_H */
