/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MEAN_H
#define MEAN_H

#include <vector>
#include "CachedComponent.h"

namespace Tracter
{
    enum MeanType
    {
        MEAN_FIXED,
        MEAN_STATIC,
        MEAN_ADAPTIVE
    };

    extern const StringEnum cMeanType[];

    /**
     * Calculates the mean (over time) of the input stream, typically
     * for Cepstral Mean Normalisation.
     */
    class Mean : public CachedComponent<float>
    {
    public:
        Mean(Component<float>* iInput, const char* iObjectName = "Mean");
        virtual ~Mean() throw() {}
        virtual void reset(bool iPropagate);
        void setTimeConstant(float iSeconds);

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

        void dotHook()
        {
            CachedComponent<float>::dotHook();
            dotRecord(1, "pole=%.2f", mPole);
            dotRecord(1, "type=%s", cMeanType[mMeanType].str);
        }

    private:
        Component<float>* mInput;
        bool mValid;
        bool mPersistent;
        MeanType mMeanType;
        std::vector<float> mPrior;
        std::vector<float> mMean;

        float mPole;
        float mElop;

        void processAll();
        bool adaptFrame(IndexType iIndex);

        void load(
            std::vector<float>& iVector,
            const char* iToken,
            const char* iFileName
        );
    };
}

#endif /* MEAN_H */
