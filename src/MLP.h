/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MLP_H
#define MLP_H

#include <vector>

#include "CachedComponent.h"
#include "SKMLP.h"

namespace Tracter
{
    /**
     * Torch based MLP
     */
    class MLP : public CachedComponent<float>
    {
    public:
        MLP(Component<float>* iInput, const char* iObjectName = "MLP");
        virtual ~MLP() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

        void DotHook()
        {
            CachedComponent<float>::DotHook();
            DotRecord(1, "win=%d=%d+1+%d", mWindow, mTheta, mTheta);
        }

    private:
        Component<float>* mInput;
        int mTheta;
        int mWindow;
        int mInputs;
        std::vector<float> mFeature;
        Torch::SKMLP mMLP;
    };
}

#endif /* MLP_H */
