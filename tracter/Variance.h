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

    extern const StringEnum cVarianceType[];

    /**
     * Calculates the variance (over time) of the input stream, typically for
     * Cepstral Variance Normalisation.
     */
    class Variance : public CachedComponent<float>
    {
    public:
        Variance(Component<float>* iInput,
                 const char* iObjectName = "Variance");
        virtual void reset(bool iPropagate);
        void setTimeConstant(float iSeconds);

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

        void dotHook()
        {
            CachedComponent<float>::dotHook();
            dotRecord(1, "pole=%.2f", mPole);
            dotRecord(1, "type=%s", cVarianceType[mVarianceType].str);
        }

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

        void load(
            std::vector<float>& iVariance,
            const char* iToken,
            const char* iFileName
        );
    };
}

#endif /* VARIANCE_H */
