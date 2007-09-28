#include <math.h>
#include <stdlib.h>
#include "MelFilter.h"

float MelFilter::hertzToMel(float iHertz)
{
    return 2595.0f * log10f(1.0f + iHertz / 700.0f);
}

float MelFilter::melToHertz(float iMel)
{
    return 700.0 * (powf(10, iMel / 2595.0f) - 1.0f);
}

int MelFilter::hertzToBin(float iHertz, int iNBins, float iHiHertz)
{
    return iHertz / iHiHertz * (iNBins-1) + 0.5;
}

MelFilter::MelFilter(
    Plugin<float>* iInput, int iNBins, float iSampleRate, float iLoHertz, float iHiHertz
)
    : UnaryPlugin<float, float>(iInput, iNBins)
{
    MinSize(mInput, 1);

    // Initialise the transform
    mWeight.resize(iNBins);
    mBin.resize(iNBins+2);
    float loMel = hertzToMel(iLoHertz);
    float hiMel = hertzToMel(iHiHertz);
    int nDFT = mInput->GetArraySize();
    float sampleRate2 = iSampleRate / 2;
    for (int i=0; i<iNBins+2; i++)
    {
        float hertz = melToHertz(loMel + (hiMel - loMel) / (iNBins + 1) * i);
        mBin[i] = hertzToBin(hertz, nDFT, sampleRate2);
    }

    for (int i=1; i<=iNBins; i++)
    {
        mWeight[i-1].resize(mBin[i+1] - mBin[i-1] + 1);

        // These bins are from ETSI.
        // They are wide - they overlap the centers either side
        int width1 = mBin[i] - mBin[i-1];
        for (int j=0; j<width1; j++)
            mWeight[i-1][j] = (float)(j+1) / (width1+1);
        int width2 = mBin[i+1] - mBin[i];
        for (int j=0; j<=width2; j++)
            mWeight[i-1][width1+j] = 1.0f - (float)j / (width1+1);
    }
}

bool MelFilter::ProcessFrame(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);

    CacheArea inputArea;
    int one = mInput->Read(inputArea, iIndex);
    if (!one)
        return false;

    float* p = mInput->GetPointer(inputArea.offset);
    float* c = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
    {
        c[i] = 0.0f;
        for (int j=0; j<mBin[i+2]-mBin[i]+1; j++)
            c[i] += mWeight[i][j] * p[mBin[i]+j];
    }

    return true;
}
