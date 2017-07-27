/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>

#include "LPCepstrum.h"

Tracter::LPCepstrum::LPCepstrum(
    Component<float>* iInput, const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(mInput);

    mNCompressed = mInput->frame().size;
    mC0 = config("C0", 1);
    mNCepstra = config("NCepstra", 12);
    mFrame.size = mC0 ? mNCepstra+1 : mNCepstra;
    assert(mFrame.size > 0);

    mOrder = config("Order", 14);
    mCompressionPower = config("CompressionPower", 0.33f);
    mRidge = config("Ridge", 0.0f);

    // We get as many autocorrelation coeffs as input dimensions
    if (mOrder >= mNCompressed)
        throw Exception("%s: Order(%d) must be less than input dimension(%d)",
                        objectName(), mOrder, mNCompressed);

    mAlpha0.resize(mOrder);
    mAlpha1.resize(mOrder);
    mCompressed = 0;
    mAutoCorrelation = 0;
    mFourier.Init(mNCompressed, &mCompressed, &mAutoCorrelation);
}

bool Tracter::LPCepstrum::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    if (mInput->Read(inputArea, iIndex) < 1)
        return false;

    // Copy the frame though a compression function
    float* p = mInput->getPointer(inputArea.offset);
    for (int i=0; i<mNCompressed; i++)
        mCompressed[i] = powf(p[i], mCompressionPower);

    // Do the DCT
    mFourier.Transform();

    // Levinson / Durbin recursion
    // Indexes are C style from 0, but the books use 1
    mAlpha0.assign(mOrder, 0.0f);
    mAlpha1.assign(mOrder, 0.0f);
    float* a0 = &mAlpha0.front();  // Current alphas
    float* a1 = &mAlpha1.front();  // Previous alphas
    float error = mAutoCorrelation[0] * (mRidge + 1.0f);

    if (error < 1e-8f)
    {
        verbose(2, "error too small at index %ld\n", iIndex);
        return bailOut(oData);
    }

    for (int i=0; i<mOrder; i++)
    {
        float* tmp = a0; a0 = a1; a1 = tmp; // Swap a1 and a0

        float sum = mAutoCorrelation[i+1];
        for (int j=0; j<i; j++)
            sum -= a1[j] * mAutoCorrelation[i-j];
        a0[i] = sum / error;
        if (!std::isfinite(a0[i]))
        {
            verbose(2, "a0[%d] = %f at index %ld\n", i, a0[i], iIndex);
            return bailOut(oData);
        }
        error *= 1.0f - a0[i] * a0[i];
        assert(std::isfinite(error));
        assert(error != 0.0f);

        for (int j=0; j<i; j++)
            a0[j] = a1[j] - a0[i] * a1[i-j-1];
    }

    // Gain (squared)
    float gain = mAutoCorrelation[0];
    for (int j=0; j<mOrder; j++)
        gain -= a0[j] * mAutoCorrelation[j+1];

#if 0
    // Compute LP power spectrum
    for (int i=0; i<mNCepstra; i++)
    {
        float omega = (float)M_PI * i/mNCepstra;
        float c = 0;
        float s = 0;
        for (int j=0; j<mOrder; j++)
        {
            c += a0[j] * cosf(omega*(j+1));
            s += a0[j] * sinf(omega*(j+1));
        }
        c = 1.0f - c;

        //oData[i] = gain / (s*s + c*c);
        oData[i] = 1.0f / (s*s + c*c);
    }
#else
    // Compute LP cepstrum replacing unknown coeffs with 0
    for (int i=0; i<mNCepstra; i++)
    {
        float sum = 0.0f;
        for (int k=0; k<i; k++)
        {
            int index = i-k-1;
            if (index < mOrder)
                sum += a0[i-k-1] * oData[k] * (k+1);
        }
        oData[i] = sum / (i+1);
        if (i < mOrder)
            oData[i] += a0[i];
        assert(std::isfinite(oData[i]));
    }

    if (mC0)
        oData[mNCepstra] = logf(std::max(gain, 1e-8f));
#endif

    return true;
}

/*
 * Something went wrong.  Just write something plausible and escape. 
 */
bool Tracter::LPCepstrum::bailOut(float* oData)
{
    for (int i=0; i<mNCepstra; i++)
        oData[i] = 0;
    if (mC0)
        oData[mNCepstra] = logf(1e-8f);

    return true;
}
