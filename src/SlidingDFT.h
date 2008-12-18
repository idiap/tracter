/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SLIGINGDFT_H
#define SLIGINGDFT_H

#include <complex>
typedef std::complex<float> complex;

/**
 * Sliding DFT
 *
 * An efficient way of calculating a single bin of a DFT.
 */
class SlidingDFT
{
public:
    void SetRotation(int iBin, int iNBins);
    const complex& Transform(float iNew, float iOld);
    void Reset()
    {
        mState.real() = 0.0f;
        mState.imag() = 0.0f;
    }

private:
    complex mState;
    complex mRotation;
};

#endif /* SLIGINGDFT_H */
