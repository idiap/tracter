/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VARIANCE_H
#define VARIANCE_H

#include <vector>
#include "UnaryPlugin.h"

namespace Tracter
{
    enum VarianceType
    {
        VARIANCE_STATIC,
        VARIANCE_ADAPTIVE
    };

    /**
     * Calculates the variance (over time) of the input stream, typically for
     * Cepstral Variance Normalisation.
     */
    class Variance : public UnaryPlugin<float, float>
    {
    public:
        Variance(Plugin<float>* iInput, const char* iObjectName = "Variance");
        virtual ~Variance() throw() {}
        virtual void Reset(bool iPropagate);
        void SetTimeConstant(float iSeconds);

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        bool mValid;
        VarianceType mVarianceType;
        std::vector<float> mVariance;

        int mBurnIn;
        int mAdaptStart;
        float mPole;
        float mElop;

        void processAll();
        bool adaptFrame(IndexType iIndex);
    };
}

#endif /* VARIANCE_H */
