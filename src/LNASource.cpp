#include <math.h>
#include <stdlib.h>

#include "LNASource.h"

/** Constructor */
LNASource::LNASource(const char* iObjectName)
    : CachedPlugin<float>()
{
    mObjectName = iObjectName;
    mArraySize = GetEnv("ArraySize", 27);
    mSampleFreq = GetEnv("SampleFreq", 8000.0f);
    mSamplePeriod = GetEnv("SamplePeriod", 80);
    assert(mArraySize > 0);

    mMapData = 0;
    mLNA16 = GetEnv("LNA16", 0);

    /* Portability check */
    assert(sizeof(unsigned short) == 2);
    assert(sizeof(unsigned char)  == 1);
}


/**
 * Maps the LNA file
 */
void LNASource::Open(const char* iFileName)
{
    assert(iFileName);
    mMapData = mMap.Map(iFileName);
    mMapSize = mMap.GetSize() / (mArraySize+1);
    if (mLNA16)
        mMapSize /= 2;
    printf("LNA Size %d\n", mMapSize);
}

/**
 * The fetch call is necessary to convert from integer to floating
 * point form.
 */
bool LNASource::UnaryFetch(IndexType iIndex, int iOffset)
{
    if (iIndex >= mMapSize)
        return false;
    float* cache = GetPointer(iOffset);
    float sum = 0.0;
    int eos;
    if (mLNA16)
    {
        /* 16 bit specific code */
        unsigned short* data = (unsigned short*)mMapData;
        data += (mArraySize+1) * iIndex;
        eos = (int)*data++;
        for (int j=0; j<mArraySize; j++)
        {
            cache[j] = -((float)data[j] + 0.5f) / 5120.0f;
            sum += expf(cache[j]);
        }
    }
    else
    {
        /* 8 bit specific code */
        unsigned char* data = (unsigned char*)mMapData;
        data += (mArraySize+1) * iIndex;
        eos = (int)*data++;
        for (int j=0; j<mArraySize; j++)
        {
            cache[j] = -((float)data[j] + 0.5f) / 24.0f;
            sum += expf(cache[j]);
        }
    }

    /* Check the sum is close to unity */
    if ((sum < 0.97) || (sum > 1.03))
    {
        printf("LNASource: Checksum error at index %ld\n", iIndex);
        exit(EXIT_FAILURE);
    }

    /* Check the eos value is sensible */
    if ( ((iIndex <  mMapSize-1) && (eos != 0x00)) ||
         ((iIndex == mMapSize-1) && (eos != 0x80)) )
    {
        printf("LNASource: EOS marker error (%d) at index %ld\n", eos, iIndex);
        exit(EXIT_FAILURE);
    }

    return true;
}
