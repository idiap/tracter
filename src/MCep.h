/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MCEP_H
#define MCEP_H

#include "UnaryPlugin.h"

namespace Tracter
{
    /**
     * Wrapper for SPTK's mcep
     */
    class MCep : public UnaryPlugin<float, float>
    {
    public:
        MCep(Plugin<float>* iInput, const char* iObjectName = "MCep");
        virtual ~MCep() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        std::vector<double> mInData;
        std::vector<double> mCepstra;
        bool mC0;
        double mAlpha;
        int mMinIter;
        int mMaxIter;
        double mEndCondition;
        double mSmall;
        double mDetMin;
    };
}

#endif /* MCEP_H */
