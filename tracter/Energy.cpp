/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "Energy.h"

Tracter::Energy::Energy(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(iInput);
}

bool Tracter::Energy::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame
    const float* p = mInput->unaryRead(iIndex);
    if (!p)
        return false;

    // Calculate energy
    *oData = 0.0f;
    for (int i=0; i<mInput->frame().size; i++)
        *oData += p[i] * p[i];

    // Done
    verbose(4, "plot %ld %e\n", iIndex, 10.0*log10(*oData));
    return true;
}
