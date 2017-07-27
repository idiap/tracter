/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cmath>

#include "MelFilter.h"

/** Convert a value in Hertz to the mel scale. */
float Tracter::MelFilter::hertzToMel(float iHertz)
{
    return 2595.0f * log10f(1.0f + iHertz / 700.0f);
}

/** Convert a value from the mel scale to Hertz. */
float Tracter::MelFilter::melToHertz(float iMel)
{
    return 700.0 * (powf(10, iMel / 2595.0f) - 1.0f);
}

/** Convert a value in Hertz to its closest periodogram bin. */
int Tracter::MelFilter::hertzToBin(float iHertz, int iNBins)
{
    return (int)(iHertz / mMaxHertz * (iNBins-1) + 0.5);
}

/** Convert a periodogram bin to its center frequency. */
float Tracter::MelFilter::binToHertz(int iBin, int iNBins)
{
    return (float)iBin * mMaxHertz / (float)(iNBins-1);
}

Tracter::MelFilter::MelFilter(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(mInput);

    mMaxHertz = config("MaxHertz", 4000.0f);
    mFrame.size = config("NBins", 23);
    mLoHertz = config("LoHertz", 64.0f);
    mHiHertz = config("HiHertz", mMaxHertz);
    mLoWarp = config("LoWarp", 0.0f);
    mHiWarp = config("HiWarp", mHiHertz * 0.85f);  // 4000 -> 3400
    mAlpha = config("Alpha", 1.0f);

    // Initialise the transform
    mWeight.resize(mFrame.size);
#ifdef ALIGNED_BINS
    initAlignedBins();
#else
    initSmoothBins();
#endif

    if (config("Normalise", 0))
        normaliseBins();
}

bool Tracter::MelFilter::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);

    CacheArea inputArea;
    int one = mInput->read(inputArea, iIndex);
    if (!one)
        return false;

    float* p = mInput->getPointer(inputArea.offset);
    for (int i=0; i<mFrame.size; i++)
    {
        //assert(mBin[i+2]-mBin[i]+1 == mWeight[i].size());
        oData[i] = 0.0f;
        for (size_t j=0; j<mWeight[i].size(); j++)
            oData[i] += mWeight[i][j] * p[mBin[i]+j];
    }

    return true;
}

/**
 * Bins where each bin center is aligned with a periodogram bin, so
 * they actually look like triangles.  Works OK, but the spacing of
 * the lower frequency bins is probably wrong because the DFT
 * granularity is not fine enough.  It's not clear that it would work
 * with VTLN at all.  It's based on a description in the ETSI advanced
 * DSR frontend, and the bins seem to be a little wide in that the
 * triangles overlap a little at the bottom.
 */
void Tracter::MelFilter::initAlignedBins()
{
    assert(mFrame.size > 0);

    // Triangles aligned with bins
    float loMel = hertzToMel(mLoHertz);
    float hiMel = hertzToMel(mHiHertz);
    int nPSD = mInput->frame().size;
    mBin.resize(mFrame.size+2);
    for (int i=0; i<mFrame.size+2; i++)
    {
        float hertz =
            melToHertz(loMel + (hiMel - loMel) / (mFrame.size + 1) * i);
        if (mAlpha != 1.0)
            hertz = warpHertz(hertz, mAlpha);
        mBin[i] = hertzToBin(hertz, nPSD);
    }

    for (int i=1; i<=mFrame.size; i++)
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

/**
 * Bins that are smoothly spaced in the mel domain, but do not
 * necessarily align with periodogram bins.  This gives bins that are
 * not obviously triangular, but are properly spaced and probably
 * respond better to VTLN.  This is similar to the way HTK does it.
 */
void Tracter::MelFilter::initSmoothBins()
{
    assert(mFrame.size > 0);

    std::vector<float> hertz;
    hertz.resize(mFrame.size+2);
    mBin.resize(mFrame.size);
    float loMel = hertzToMel(mLoHertz);
    float hiMel = hertzToMel(mHiHertz);
    int nPSD = mInput->frame().size;

    // Get a list of mel bin centers in hertz
    for (int i=0; i<mFrame.size+2; i++)
    {
        hertz[i] = melToHertz(loMel + (hiMel - loMel) / (mFrame.size + 1) * i);
        if (mAlpha != 1.0)
            hertz[i] = warpHertz(hertz[i], mAlpha);
    }

    for (int p=0; p<nPSD; p++)
    {
        float centre = binToHertz(p, nPSD);
        if (centre < hertz[0])
            continue;

        for (int m=0; m<mFrame.size; m++)
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

/**
 * Normalise the bins to have unity power.  It's not clear that this
 * is necessary at all; it breaks the triangular thing where each DFT
 * bin's contribution is unity.  Further, this type of scaling (or
 * not) just appears as a constant offset in cepstral space.  Cepstral
 * mean normalisation will remove it.
 */
void Tracter::MelFilter::normaliseBins()
{
    assert(mFrame.size > 0);

    // Normalise filters
    for (int m=0; m<mFrame.size; m++)
    {
        float sum = 0.0f;
        for (size_t w=0; w<mWeight[m].size(); w++)
            sum += mWeight[m][w];
        for (size_t w=0; w<mWeight[m].size(); w++)
            mWeight[m][w] /= sum;
    }
}

/**
 * Warp a value in Hertz by wavelength scale alpha.  This is based on
 * the HTK 'kite' picture for VTLN.
 */
float Tracter::MelFilter::warpHertz(
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

    if (warp < mLoHertz)
        // ...which is very unlikely
        throw Exception("MelFilter: warp (%f) < mLoHertz (%f)\n",
                        warp, mLoHertz);
    if (warp > mHiHertz * 1.00001) // to allow for accumulated errors
        // Over-zealous alpha?
        throw Exception("MelFilter: warp (%f) > mHiHertz (%f)\n",
                        warp, mHiHertz);

    return warp;
}

/**
 * Dumps the bin weights to gnuplot's (trivial) data format so they
 * can be plotted.
 */
void Tracter::MelFilter::DumpBins()
{
    // Build a fully expanded array
    std::vector< std::vector<float> > output;
    int psdSize = mInput->frame().size;
    output.resize(psdSize);
    for (int i=0; i<psdSize; i++)
    {
        output[i].resize(mFrame.size);
        for (int j=0; j<mFrame.size; j++)
            output[i][j] = 0.0f;
    }
    for (int j=0; j<mFrame.size; j++)
        for (size_t k=0; k<mWeight[j].size(); k++)
        {
            assert((int)k<psdSize);
            output[mBin[j]+k][j] = mWeight[j][k];
        }

    // Dump it
    for (int i=0; i<psdSize; i++)
    {
        printf("%d", i);
        for (int j=0; j<mFrame.size; j++)
            printf(" %f", output[i][j]);
        printf("\n");
    }
}
