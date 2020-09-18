/*
 * Copyright 2010 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "OverlapAdd.h"

Tracter::OverlapAdd::OverlapAdd(
    Component<float>* iInput, const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(mInput, 2);
}

Tracter::SizeType Tracter::OverlapAdd::contiguousFetch(
    IndexType iIndex, SizeType iLength, SizeType iOffset
)
{
    assert(iIndex >= 0);

    /* The lap is the "period"; half the window size */
    int lap = mInput->frame().size / 2;
    IndexType in = iIndex / lap; // Round down!
    int fi = iIndex - (in * lap);

    /* Translate the output index into lo and hi overlapping frames */
    const float* lo = 0;
    const float* hi = 0;
    if (in > 0)
        lo = mInput->unaryRead(in-1);
    hi = mInput->unaryRead(in);

    float* op = getPointer(iOffset);
    for(int i=0; i<iLength; i++)
    {
        /* End? */
        if (!hi && !lo)
            return i;

        /* Add the lo and hi overlapping elements */
        float sum = 0.0f;
        if (lo)
            sum += lo[lap+fi];
        if (hi)
            sum += hi[fi];
        op[i] = sum;

        /* Get a new frame if we're at the end */
        if (++fi >= lap)
        {
            fi = 0;
            lo = hi;
            hi = mInput->unaryRead(++in);
        }
    }

    return iLength;
}
