/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PIXMAP_H
#define PIXMAP_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Writes a Pixmap (image) representing the input
     */
    class Pixmap : public CachedComponent<float>
    {
    public:
        Pixmap(Component<float>* iInput, const char* iObjectName = "Pixmap");

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        void write();

        Component<float>* mInput;
        int mLoIndex;
        int mHiIndex;
        float mMin;
        float mMax;
        float mRange;
        bool mLog;
    };
}

#endif /* PIXMAP_H */
