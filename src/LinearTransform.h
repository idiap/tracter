/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef LINEARTRANSFORM_H
#define LINEARTRANSFORM_H

#include <vector>
#include "UnaryPlugin.h"

namespace Tracter
{
    class LinearTransform : public UnaryPlugin<float, float>
    {
    public:
        LinearTransform(Plugin<float>* iInput,
                        const char* iObjectName = "LinearTransform");
        virtual ~LinearTransform() throw() {}

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);
        int LoadXForm(const char* iFileName);

    private:
        std::vector<float> mMatrix;
    };
}

#endif /* LINEARTRANSFORM_H */
