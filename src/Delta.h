/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef DELTA_H
#define DELTA_H

#include <vector>

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Component to calculate delta features
     */
    class Delta : public CachedComponent<float>
    {
    public:
        Delta(Component<float>* iInput, const char* iObjectName = "Delta");
        virtual ~Delta() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        int mTheta;
        int mWindow;
        std::vector<float*> mFeature;
        std::vector<float> mWeight;
    };
}

#endif /* DELTA_H */
