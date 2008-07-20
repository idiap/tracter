/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef DELTA_H
#define DELTA_H

#include <vector>

#include "UnaryPlugin.h"

namespace Tracter
{
    /**
     * Plugin to calculate delta features
     */
    class Delta : public UnaryPlugin<float, float>
    {
    public:
        Delta(Plugin<float>* iInput, const char* iObjectName = "Delta");
        virtual ~Delta() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        int mTheta;
        int mWindow;
        std::vector<float*> mFeature;
        std::vector<float> mWeight;
    };
}

#endif /* DELTA_H */
