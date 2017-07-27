/*
 * Copyright 2007 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstring>

#include "Normalise.h"

Tracter::Normalise::Normalise(
    Component<short>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    mFrame.size = mInput->frame().size;
    connect(mInput);

    Endian endian = (Endian)config(cEndian, ENDIAN_NATIVE);
    mByteOrder.setSource(endian);
}

void Tracter::Normalise::minSize(
    SizeType iSize, SizeType iReadBehind, SizeType iReadAhead
)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    ComponentBase::minSize(iSize, iReadBehind, iReadAhead);

    // We expect the input buffer to be at least the size of each request
    assert(mInput);
    ComponentBase::minSize(mInput, iSize, 0, 0);
}

Tracter::SizeType
Tracter::Normalise::fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);
    assert(mFrame.size);
    CacheArea inputArea;
    SizeType lenGot = mInput->read(inputArea, iIndex, iOutputArea.length());
    short* input = mInput->getPointer(inputArea.offset);
    float* output = getPointer(iOutputArea.offset);

    SizeType rOffset = 0;
    SizeType wOffset = 0;
    for (SizeType i=0; i<lenGot; i++)
    {
        if (i == inputArea.len[0])
        {
            input = mInput->getPointer(0);
            rOffset = 0;
        }
        if (i == iOutputArea.len[0])
        {
            output = getPointer(0);
            wOffset = 0;
        }

        if (mByteOrder.wrongEndian())
            for (int j=0; j<mFrame.size; j++)
            {
                // Inefficient!
                short s = input[rOffset++];
                mByteOrder.swap(&s, 2, 1);
                output[wOffset++] = (float)s / 32768.0f;
            }
        else
            for (int j=0; j<mFrame.size; j++)
            {
                output[wOffset++] = (float)input[rOffset++] / 32768.0f;
            }
    }

    return lenGot;
}
