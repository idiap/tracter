/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "FrequencyWarp.h"

/** Convert a value in Hertz to the warp scale. */
float Tracter::FrequencyWarp::hertzToWarp(float iHertz)
{
    return 2595.0f * log10f(1.0f + iHertz / 700.0f);
}

/** Convert a value from the warp scale to Hertz. */
float Tracter::FrequencyWarp::warpToHertz(float iWarp)
{
    return 700.0 * (powf(10, iWarp / 2595.0f) - 1.0f);
}

/** Convert a value in Hertz to its closest periodogram bin. */
int Tracter::FrequencyWarp::hertzToBin(float iHertz)
{
    return (int)(iHertz / mMaxHertz * (mNPSD-1) + 0.5);
}

/** Convert a periodogram bin to its center frequency. */
float Tracter::FrequencyWarp::binToHertz(int iBin)
{
    return (float)iBin * mMaxHertz / (float)(mNPSD-1);
}


Tracter::FrequencyWarp::FrequencyWarp(const char* iObjectName)
{
    mObjectName = iObjectName;

    mMaxHertz = GetEnv("MaxHertz", 4000.0f);
    mNBins = GetEnv("NBins", 23);
    mLoHertz = GetEnv("LoHertz", 64.0f);
    mHiHertz = GetEnv("HiHertz", mMaxHertz);
    mLoWarp = GetEnv("LoWarp", 0.0f);
    mHiWarp = GetEnv("HiWarp", mHiHertz * 0.85f);  // 4000 -> 3400
    mAlpha = GetEnv("Alpha", 1.0f);
}


#ifdef ALIGNED_BINS

/**
 * Bins where each bin center is aligned with a periodogram bin, so
 * they actually look like triangles.  Works OK, but the spacing of
 * the lower frequency bins is probably wrong because the DFT
 * granularity is not fine enough.  It's not clear that it would work
 * with VTLN at all.  It's based on a description in the ETSI advanced
 * DSR frontend, and the bins seem to be a little wide in that the
 * triangles overlap a little at the bottom.
 */
void Tracter::FrequencyWarp::Initialise(int iNPSD)
{
    assert(iNPSD > 0);
    mNPSD = iNPSD;
    mWeight.resize(mNBins);

    // Triangles aligned with bins
    float loWarp = hertzToWarp(mLoHertz);
    float hiWarp = hertzToWarp(mHiHertz);
    mBin.resize(mNBins+2);
    for (int i=0; i<mNBins+2; i++)
    {
        float hertz =
            warpToHertz(loWarp + (hiWarp - loWarp) / (mNBins + 1) * i);
        if (mAlpha != 1.0)
            hertz = warpHertz(hertz, mAlpha);
        mBin[i] = hertzToBin(hertz);
    }

    for (int i=1; i<=mNBins; i++)
    {
        mWeight[i-1].resize(mBin[i+1] - mBin[i-1] + 1);

        // These bins are from ETSI.
        // They are wide - they overlap the centers either side
        int width1 = mBin[i] - mBin[i-1];
        for (int j=0; j<width1; j++)
            mWeight[i-1][j] = (float)(j+1) / (width1+1);
        int width2 = mBin[i+1] - mBin[i];
        for (int j=0; j<=width2; j++)
            mWeight[i-1][width1+j] = 1.0f - (float)j / (width2+1);
    }
}

#else

/**
 * Bins that are smoothly spaced in the warp domain, but do not
 * necessarily align with periodogram bins.  This gives bins that are
 * not obviously triangular, but are properly spaced and probably
 * respond better to VTLN.  This is similar to the way HTK does it.
 */
void Tracter::FrequencyWarp::Initialise(int iNPSD)
{
    assert(iNPSD > 0);
    mNPSD = iNPSD;
    mWeight.resize(mNBins);

    std::vector<float> hertz;
    hertz.resize(mNBins+2);
    mBin.resize(mNBins);
    float loWarp = hertzToWarp(mLoHertz);
    float hiWarp = hertzToWarp(mHiHertz);

    // Get a list of warp bin centers in hertz
    for (int i=0; i<mNBins+2; i++)
    {
        hertz[i] = warpToHertz(
            loWarp + (hiWarp - loWarp) / (mNBins + 1) * i
        );
        if (mAlpha != 1.0)
            hertz[i] = warpHertz(hertz[i], mAlpha);
    }

    for (int p=0; p<iNPSD; p++)
    {
        float centre = binToHertz(p);
        if (centre < hertz[0])
            continue;

        for (int m=0; m<mNBins; m++)
        {
            // Lower triangle
            if ((centre > hertz[m]) &&
                (centre < hertz[m+1]))
            {
                float weight =
                    (centre - hertz[m]) / (hertz[m+1] - hertz[m]);
                mWeight[m].push_back(weight);
                if (mWeight[m].size() == 1)
                    mBin[m] = p;
            }

            // Upper triangle
            if ((centre >= hertz[m+1]) &&
                (centre <  hertz[m+2]))
            {
                float weight =
                    (hertz[m+2] - centre) / (hertz[m+2] - hertz[m+1]);
                mWeight[m].push_back(weight);
                if (mWeight[m].size() == 1)
                    mBin[m] = p;
            }
        }
    }
}

#endif

/**
 * Normalise the bins to have unity power.  It's not clear that this
 * is necessary at all; it breaks the triangular thing where each DFT
 * bin's contribution is unity.  Further, this type of scaling (or
 * not) just appears as a constant offset in cepstral space.  Cepstral
 * mean normalisation will remove it.
 */
void Tracter::FrequencyWarp::normaliseBins()
{
    assert(mNBins > 0);

    // Normalise filters
    for (int m=0; m<mNBins; m++)
    {
        float sum = 0.0f;
        for (size_t w=0; w<mWeight[m].size(); w++)
            sum += mWeight[m][w];
        for (size_t w=0; w<mWeight[m].size(); w++)
            mWeight[m][w] /= sum;
    }
}

/**
 * Calculate and return the weight associated with each bin.
 */
float Tracter::FrequencyWarp::Weight(int iBin)
{
    float sum = 0.0f;
    for (size_t w=0; w<mWeight[iBin].size(); w++)
        sum += mWeight[iBin][w];
    return sum;
}

/**
 * Warp a value in Hertz by wavelength scale alpha.  This is based on
 * the HTK 'kite' picture for VTLN.
 */
float Tracter::FrequencyWarp::warpHertz(
    float iHertz,   ///< Value to warp
    float iAlpha    ///< Warp factor
)
{
    float scale = 1.0f/iAlpha;
    float loInt = mLoWarp*2.0 / (1.0 + scale);
    float hiInt = mHiWarp*2.0 / (1.0 + scale);
    float warp;

    assert(loInt >= 0.0f);
    assert(hiInt < mHiHertz);

    if (iHertz < loInt)
    {
        float gradient = (scale*loInt-mLoHertz) / (loInt-mLoHertz);
        warp =  gradient * (iHertz-mLoHertz) + mLoHertz;
    }
    else if (iHertz > hiInt)
    {
        float gradient = (mHiHertz-scale*hiInt) / (mHiHertz-hiInt);
        warp = gradient * (iHertz-mHiHertz) + mHiHertz;
    }
    else
        warp = scale*iHertz;

    if (warp < mLoHertz) // ...which is very unlikely
    {
        printf("FrequencyWarp: warp (%f) < mLoHertz (%f)\n", warp, mLoHertz);
        exit(EXIT_FAILURE);
    }
    if (warp > mHiHertz * 1.00001) // to allow for accumulated errors
    {
        // Over-zealous alpha?
        printf("FrequencyWarp: warp (%f) > mHiHertz (%f)\n", warp, mHiHertz);
        exit(EXIT_FAILURE);
    }

    return warp;
}

/**
 * Dumps the bin weights to gnuplot's (trivial) data format so they
 * can be plotted.
 */
void Tracter::FrequencyWarp::DumpBins()
{
    // Build a fully expanded array
    std::vector< std::vector<float> > output;
    output.resize(mNPSD);
    for (int i=0; i<mNPSD; i++)
    {
        output[i].resize(mNBins);
        for (int j=0; j<mNBins; j++)
            output[i][j] = 0.0f;
    }
    for (int j=0; j<mNBins; j++)
        for (size_t k=0; k<mWeight[j].size(); k++)
        {
            assert((int)k<mNPSD);
            output[mBin[j]+k][j] = mWeight[j][k];
        }

    // Dump it
    for (int i=0; i<mNPSD; i++)
    {
        printf("%d", i);
        for (int j=0; j<mNBins; j++)
            printf(" %f", output[i][j]);
        printf("\n");
    }
}
