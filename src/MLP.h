/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MLP_H
#define MLP_H

#include <vector>

#include "UnaryPlugin.h"
#include "SKMLP.h"

namespace Tracter
{
    /**
     * Plugin to calculate delta features
     */
    class MLP : public UnaryPlugin<float, float>
    {
    public:
        MLP(Plugin<float>* iInput, const char* iObjectName = "MLP");
        virtual ~MLP() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        int mTheta;
        int mWindow;
        int mInputs;
        std::vector<float> mFeature;
        Torch::SKMLP mMLP;
    };
}

#endif /* DELTA_H */
