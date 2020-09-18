/*
 * Copyright 2011 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2011
 */

#include "Unframe.h"

Tracter::Unframe::Unframe(Component<float>* iInput, const char* iObjectName)
{
    objectName(iObjectName);

    mFrame.period = 1.0f/iInput->frame().size;
    assert(mFrame.period > 0);

    // Framers look ahead, not back
    mInput = iInput;
    connect(mInput);
}

Tracter::SizeType
Tracter::Unframe::contiguousFetch(
    IndexType iIndex, SizeType iLength, SizeType iOffset
)
{
    assert(iIndex >= 0);

    // Limits
    int inSize = mInput->frame().size;
    IndexType loIndex = iIndex / inSize;
    IndexType hiIndex = (iIndex + iLength) / inSize;
    int loOffset = iIndex % inSize;
    int hiOffset = (iIndex + iLength) % inSize;

    verbose(3, "Reading %d from %d of %ld to %d of %ld\n",
            iLength, loOffset, loIndex, hiOffset, hiIndex);

    // First frame
    int nRead = 0;
    const float* ip = mInput->unaryRead(loIndex);
    if (!ip)
        return nRead;
    float* oData = getPointer(iOffset);
    int hi = (loIndex == hiIndex) ? hiOffset : inSize;
    for (int i=loOffset; i<hi; i++)
        oData[nRead++] = ip[i];

    // Middle frames
    for (IndexType j=loIndex+1; j<hiIndex; j++)
    {
        ip = mInput->unaryRead(j);
        if (!ip)
            return nRead;
        for (int i=0; i<inSize; i++)
            oData[nRead++] = ip[i];
    }

    // Last frame
    if (nRead != iLength)
    {
        ip = mInput->unaryRead(hiIndex);
        if (!ip)
            return nRead;
        for (int i=0; i<hiOffset; i++)
            oData[nRead++] = ip[i];
    }

    // Done
    assert(nRead == iLength);
    return nRead;
}
