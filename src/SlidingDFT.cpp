/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include "SlidingDFT.h"

void SlidingDFT::SetRotation(int iBin, int iNBins)
{
    float a = 2.0f * M_PI * iBin / iNBins;
    float r = cosf(a);
    float i = sinf(a);
    mRotation = complex(r, i);
}

const complex& SlidingDFT::Transform(float iNew, float iOld)
{
    float tmp = iNew - iOld;
    complex ctmp = mState + tmp;
    mState = mRotation * ctmp;
    return mState;
}
