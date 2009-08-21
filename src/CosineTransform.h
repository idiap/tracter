/*
 * Copyright 2009 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef COSINETRANSFORM_H
#define COSINETRANSFORM_H

#include <vector>

#include "CachedComponent.h"
#include "Fourier.h"

namespace Tracter
{
    class CosineTransform : public CachedComponent<float>
    {
    public:
        CosineTransform(
            Component<float>* iInput, const char* iObjectName = "CosineTransform"
        );
        virtual ~CosineTransform() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        Fourier mDCT;
        float* mIData;
        float* mOData;
        std::vector<float> mWindow;
    };
}

#endif /* COSINETRANSFORM_H */
