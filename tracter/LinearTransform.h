/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef LINEARTRANSFORM_H
#define LINEARTRANSFORM_H

#include <vector>
#include "CachedComponent.h"

namespace Tracter
{
    class LinearTransform : public CachedComponent<float>
    {
    public:
        LinearTransform(Component<float>* iInput,
                        const char* iObjectName = "LinearTransform");
        virtual ~LinearTransform() throw() {}

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);
        int LoadXForm(const char* iFileName);

    private:
        Component<float>* mInput;
        std::vector<float> mMatrix;
    };
}

#endif /* LINEARTRANSFORM_H */
