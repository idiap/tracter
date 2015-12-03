/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FRAME_H
#define FRAME_H

#include "CachedComponent.h"

namespace Tracter
{
    class Frame : public CachedComponent<float>
    {
    public:
        Frame(Component<float>* iInput,
               const char* iObjectName = "Frame");

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
    };
}

#endif /* FRAME_H */
