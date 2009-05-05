/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Energy.h"

Tracter::Energy::Energy(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    PluginObject::MinSize(mInput, 1);
}

bool Tracter::Energy::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    // Read the input frame
    const float* p = mInput->UnaryRead(iIndex);
    if (!p)
        return false;

    // Calculate energy
    float* energy = GetPointer(iOffset);
    *energy = 0.0f;
    for (int i=0; i<mInput->GetArraySize(); i++)
        *energy += p[i] * p[i];

    // Done
    return true;
}
