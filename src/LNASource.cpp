/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include <cassert>

#include "LNASource.h"

/** Constructor */
Tracter::LNASource::LNASource(const char* iObjectName)
{
    mObjectName = iObjectName;
    mFrame.size = GetEnv("FrameSize", 27);
    mFrameRate = GetEnv("FrameRate", 8000.0f);
    mFrame.period = GetEnv("FramePeriod", 80);
    assert(mFrame.size > 0);

    mMapData = 0;
    mLNA16 = GetEnv("LNA16", 0);
    mCheckSum = GetEnv("CheckSum", 1);

    /* Portability check */
    assert(sizeof(unsigned short) == 2);
    assert(sizeof(unsigned char)  == 1);
}


/**
 * Maps the LNA file
 */
void Tracter::LNASource::Open(
    const char* iFileName, TimeType iBeginTime, TimeType iEndTime
)
{
    assert(iFileName);
    mMapData = mMap.Map(iFileName);
    mMapSize = mMap.Size() / (mFrame.size+1);
    if (mLNA16)
        mMapSize /= 2;
    Verbose(1, "LNA Size %d\n", mMapSize);
}

/**
 * The fetch call is necessary to convert from integer to floating
 * point form.
 */
bool Tracter::LNASource::UnaryFetch(IndexType iIndex, float* oData)
{
    if (iIndex >= mMapSize)
        return false;
    float sum = 0.0;
    int eos;
    if (mLNA16)
    {
        /* 16 bit specific code */
        unsigned short* data = (unsigned short*)mMapData;
        data += (mFrame.size+1) * iIndex;
        eos = (int)*data++;
        for (int j=0; j<mFrame.size; j++)
        {
            oData[j] = -((float)data[j] + 0.5f) / 5120.0f;
            sum += expf(oData[j]);
        }
    }
    else
    {
        /* 8 bit specific code */
        unsigned char* data = (unsigned char*)mMapData;
        data += (mFrame.size+1) * iIndex;
        eos = (int)*data++;
        for (int j=0; j<mFrame.size; j++)
        {
            oData[j] = -((float)data[j] + 0.5f) / 24.0f;
            sum += expf(oData[j]);
        }
    }

    /* Check the sum is close to unity */
    if (mCheckSum)
        if ((sum < 0.97) || (sum > 1.03))
            throw Exception("LNASource: Checksum error at index %ld", iIndex);

    /* Check the eos value is sensible */
    if ( ((iIndex <  mMapSize-1) && (eos != 0x00)) ||
         ((iIndex == mMapSize-1) && (eos != 0x80)) )
        throw Exception("LNASource: EOS marker error (%d) at index %ld\n",
                        eos, iIndex);

    return true;
}
