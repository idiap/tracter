/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VARIANCE_H
#define VARIANCE_H

#include <vector>
#include "CachedComponent.h"

namespace Tracter
{
    enum VarianceType
    {
        VARIANCE_FIXED,
        VARIANCE_STATIC,
        VARIANCE_ADAPTIVE
    };

    /**
     * Calculates the variance (over time) of the input stream, typically for
     * Cepstral Variance Normalisation.
     */
    class Variance : public CachedComponent<float>
    {
    public:
        Variance(Component<float>* iInput, const char* iObjectName = "Variance");
        virtual ~Variance() throw() {}
        virtual void Reset(bool iPropagate);
        void SetTimeConstant(float iSeconds);

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        bool mValid;
        bool mPersistent;
        VarianceType mVarianceType;
        std::vector<float> mPrior;
        std::vector<float> mVariance;
        std::vector<float> mTarget;

        int mBurnIn;
        int mAdaptStart;
        float mPole;
        float mElop;

        void processAll();
        bool adaptFrame(IndexType iIndex);

        void Load(
            std::vector<float>& iVariance,
            const char* iToken,
            const char* iFileName
        );
    };
}

#endif /* VARIANCE_H */
