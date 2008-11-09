/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FRAME_H
#define FRAME_H

#include "UnaryPlugin.h"

namespace Tracter
{
    class Frame : public UnaryPlugin<float, float>
    {
    public:
        Frame(Plugin<float>* iInput,
               const char* iObjectName = "Frame");

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        int mFrameSize;
        int mFramePeriod;
    };
}

#endif /* FRAME_H */
