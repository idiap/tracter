/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef NORMALISE_H
#define NORMALISE_H

#include "UnaryPlugin.h"
#include "ByteOrder.h"

namespace Tracter
{
    /**
     * Normalises an audio input of type short to be a float between
     * -1 and 1, doing byte swapping if necessary.
     */
    class Normalise : public UnaryPlugin<float, short>
    {
    public:
        Normalise(
            Plugin<short>* iInput, const char* iObjectName = "Normalise"
        );
        void MinSize(int iSize, int iReadAhead);

    protected:
        ByteOrder mByteOrder;
        int Fetch(IndexType iIndex, CacheArea& iOutputArea);
    };
}

#endif /* NORMALISE_H */
