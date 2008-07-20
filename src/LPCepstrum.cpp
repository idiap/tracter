/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <math.h>
#include "LPCepstrum.h"

Tracter::LPCepstrum::LPCepstrum(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mNCompressed = mInput->GetArraySize();

    mC0 = GetEnv("C0", 1);
    mNCepstra = GetEnv("NCepstra", 12);
    mArraySize = mC0 ? mNCepstra+1 : mNCepstra;
    assert(mArraySize > 0);
    MinSize(mInput, 1);

    mOrder = GetEnv("Order", 14);
    mCompressionPower = GetEnv("CompressionPower", 0.33f);

    // We get as many autocorrelation coeffs as input dimensions
    if (mOrder >= mNCompressed)
    {
        printf("%s: Order (%d) must be less than input dimension (%d)\n",
               mObjectName, mOrder, mNCompressed);
        exit(EXIT_FAILURE);
    }

    mCompressed = 0;
    mAutoCorrelation = 0;
    mFourier.Init(mNCompressed, &mCompressed, &mAutoCorrelation);
}

bool Tracter::LPCepstrum::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Read the input frame
    if (mInput->Read(inputArea, iIndex) < 1)
        return false;

    // Copy the frame though a compression function
    float* p = mInput->GetPointer(inputArea.offset);
    for (int i=0; i<mNCompressed; i++)
        mCompressed[i] = powf(p[i], mCompressionPower);

    // Do the DCT
    mFourier.Transform();

    // Levinson / Durbin recursion
    // Indexes are C style from 0, but the books use 1
    std::vector<float> alpha0(mOrder);
    std::vector<float> alpha1(mOrder);
    float* a0 = &alpha0[0];  // Current alphas
    float* a1 = &alpha1[0];  // Previous alphas
    for (int i=0; i<mOrder; i++)
    {
        a0[i] = 0.0f;
        a1[i] = 0.0f;
    }
    float error = mAutoCorrelation[0];
    for (int i=0; i<mOrder; i++)
    {
        float* tmp = a0; a0 = a1; a1 = tmp; // Swap a1 and a0
        float sum = 0.0f;
        for (int j=0; j<i; j++)
            sum += a1[j] * mAutoCorrelation[i-j];
        float parcor = (mAutoCorrelation[i+1] - sum ) / error;
        a0[i] = parcor;
        for (int j=0; j<i; j++)
            a0[j] = a1[j] - parcor * a1[i-j-1];
        error *= (1.0f - parcor * parcor);
    }

    // Gain (squared)
    float gain = mAutoCorrelation[0];
    for (int j=0; j<mOrder; j++)
        gain -= a0[j] * mAutoCorrelation[j+1];

#if 0
    // Compute LP power spectrum
    float* cache = GetPointer(iOffset);
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

        //cache[i] = gain / (s*s + c*c);
        cache[i] = 1.0f / (s*s + c*c);
    }
#else
    // Compute LP cepstrum replacing unknown coeffs with 0
    float* cache = GetPointer(iOffset);
    for (int i=0; i<mNCepstra; i++)
    {
        float sum = 0.0f;
        for (int k=0; k<i; k++)
        {
            int index = i-k-1;
            if (index < mOrder)
                sum += a0[i-k-1] * cache[k] * (k+1);
        }
        cache[i] = sum / (i+1);
        if (i < mOrder)
            cache[i] += a0[i];
    }

    if (mC0)
        cache[mNCepstra] = logf(gain);
#endif

    return true;
}
