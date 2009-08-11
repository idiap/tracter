/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SELECT_H
#define SELECT_H

#include "UnaryPlugin.h"

namespace Tracter
{
    /**
     * Selects a sub-array
     */
    class Select : public UnaryPlugin<float, float>
    {
    public:
      Select(Plugin<float>* iInput, const char* iObjectName = "Select");

    protected:
      bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
      int mLoIndex;
      int mHiIndex;

    };
}

#endif /* SELECT_H */
