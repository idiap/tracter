/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdlib.h>
#include "HTKSource.h"

HTKSource::HTKSource(const char* iObjectName)
    : CachedPlugin<float>()
{
    mObjectName = iObjectName;
    mArraySize = GetEnv("ArraySize", 39);
    mSampleFreq = GetEnv("SampleFreq", 8000.0f);
    mSamplePeriod = GetEnv("SamplePeriod", 80);

    mMapData = 0;
    mNSamples = 0;
    mByteOrder.SetSource(ENDIAN_BIG);
}


/**
 * Maps the HTK parameter file and reads the header
 */
void HTKSource::Open(const char* iFileName)
{
    assert(iFileName);
    mMapData = (float*)mMap.Map(iFileName);

    /*
     * Header is of the form:
     *
     * nSamples   4
     * sampPeriod 4
     * sampSize   2
     * parmKind   2
     *
     * So we handle each one separately, beginning with a portability
     * check.
     */
    assert(sizeof(float) == 4);
    assert(sizeof(int)   == 4);
    assert(sizeof(short) == 2);
    int nSamples;
    int sampPeriod;
    short sampSize;
    short parmKind;

    char* data = (char*)mMapData;
    assert(data);

    nSamples = *(int*)data;
    if (mByteOrder.WrongEndian())
        mByteOrder.Swap(&nSamples, 4, 1);
    data += 4;

    sampPeriod = *(int*)data;
    if (mByteOrder.WrongEndian())
        mByteOrder.Swap(&sampPeriod, 4, 1);
    data += 4;

    sampSize = *(short*)data;
    if (mByteOrder.WrongEndian())
        mByteOrder.Swap(&sampSize, 2, 1);
    data += 2;

    parmKind = *(short*)data;
    if (mByteOrder.WrongEndian())
        mByteOrder.Swap(&parmKind, 2, 1);
    data += 2;

    // This comparison is a little tricky as it's floating point
    float objPeriod = mSamplePeriod / mSampleFreq;
    float htkPeriod = sampPeriod * 1e-7;
    float subPeriod = htkPeriod - objPeriod;
    if ( (subPeriod >  1e-9) ||
         (subPeriod < -1e-9) )
    {
        printf("HTKSource: sample period %f not equal to expected period %f\n",
               objPeriod, htkPeriod);
        exit(EXIT_FAILURE);
    }

    if (sampSize/4 != mArraySize)
    {
        printf("HTKSource: sample size %d/4 not equal to expected size %d\n",
               sampSize, mArraySize);
        exit(EXIT_FAILURE);
    }

    if ((size_t)nSamples * sampSize + 12 != mMap.GetSize())
    {
        printf("HTKSource: data size in header not equal to size in file\n");
        exit(EXIT_FAILURE);
    }

    printf("nSamples: %d  parm: %ho\n", nSamples, parmKind);
    mNSamples = nSamples;
    mMapData = (float*)data;
}

/**
 * The Fetch call is necessary to byte swap the data
 */
int HTKSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    int i;
    int offset = iOutputArea.offset;
    for (i=0; i<iOutputArea.Length(); i++)
    {
        if (iIndex >= mNSamples)
            break;
        if (i == iOutputArea.len[0])
            offset = 0;
        float* cache = GetPointer(offset);
        for (int j=0; j<mArraySize; j++)
            cache[j] = mMapData[iIndex*mArraySize + j];
        if (mByteOrder.WrongEndian())
            mByteOrder.Swap(cache, 4, mArraySize);
        iIndex++;
        offset++;
    }
    return i;
}
