/*
 * Copyright 2011 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2011
 */

#include <cstdio>
#include <cmath>

#include "Comparator.h"

Tracter::Comparator::Comparator(
    Component<float>* iInput1,
    Component<float>* iInput2,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput1 = iInput1;
    mInput2 = iInput2;
    Connect(iInput1);
    Connect(iInput2);

    mShowGuts = GetEnv("ShowGuts", 0);

    // Threshold
    float dBThres = GetEnv("Threshold", 0.0f);
    mThreshold = powf(10.0f, dBThres / 10.0f);
}

bool Tracter::Comparator::UnaryFetch(IndexType iIndex, BoolType* oData)
{
    Verbose(3, "iIndex %ld\n", iIndex);

    /* Read and compare */
    const float* input1 = mInput1->UnaryRead(iIndex);
    if (!input1)
        return false;
    const float* input2 = mInput2->UnaryRead(iIndex);
    if (!input2)
        return false;

    BoolType state = (*input1 > *input2 * mThreshold) ? true : false;
    *oData = state;

    /* Feedback */
    if (sVerbose >= 2) // ie, dont always do the log calculations
        Verbose(2, "plot %ld %e %e %e %d\n",
                iIndex,
                10.0*log10f(*input1),
                10.0*log10f(*input2),
                10.0*log10f(*input2 * mThreshold),
                (int)(state ? 0 : -10)
        );

    return true;
}
