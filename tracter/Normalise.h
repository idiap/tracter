/*
 * Copyright 2007 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef NORMALISE_H
#define NORMALISE_H

#include "CachedComponent.h"
#include "ByteOrder.h"

namespace Tracter
{
    /**
     * Normalises an audio input of type short to be a float between
     * -1 and 1, doing byte swapping if necessary.
     */
    class Normalise : public CachedComponent<float>
    {
    public:
        Normalise(
            Component<short>* iInput, const char* iObjectName = "Normalise"
        );
        void minSize(SizeType iSize, SizeType iReadBehind, SizeType iReadAhead);

        void dotHook()
        {
            CachedComponent<float>::dotHook();
            dotRecord(1, "swap=%s", mByteOrder.wrongEndian() ? "yes" : "no");
        }

    protected:
        Component<short>* mInput;
        ByteOrder mByteOrder;
        SizeType fetch(IndexType iIndex, CacheArea& iOutputArea);
    };
}

#endif /* NORMALISE_H */
