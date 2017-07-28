/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MCEP_H
#define MCEP_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Wrapper for SPTK's mcep and mgcep
     *
     * If Gamma > 0 then mcep() is run (it's a non-sensical default;
     * mcep() actually has no Gamma parameter).  Otherwise mgcep() is
     * run.  In the latter case, if Gamma != 0 then mgc2mgc is also
     * run to set Gamma to 0.  The output is then always cepstrum.
     */
    class MCep : public CachedComponent<float>
    {
    public:
        MCep(Component<float>* iInput, const char* iObjectName = "MCep");

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        std::vector<double> mInData;
        std::vector<double> mCepstra1;
        std::vector<double> mCepstra2;
        bool mC0;
        double mAlpha;
        double mGamma;
        int mMinIter;
        int mMaxIter;
        double mEndCondition;
        double mSmall;
        double mDetMin;
    };
}

#endif /* MCEP_H */
