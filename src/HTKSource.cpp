/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdlib>

#include "HTKSource.h"

Tracter::HTKSource::HTKSource(const char* iObjectName)
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
void Tracter::HTKSource::Open(
    const char* iFileName, TimeType iBeginTime, TimeType iEndTime
)
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
        throw Exception("HTKSource:"
                        " sample period %f not equal to expected period %f",
                        objPeriod, htkPeriod);

    if (sampSize/4 != mArraySize)
        throw Exception("HTKSource:"
                        " sample size %d/4 not equal to expected size %d\n",
                        sampSize, mArraySize);

    if ((size_t)nSamples * sampSize + 12 != mMap.GetSize())
        throw Exception("HTKSource:"
                        " data size in header not equal to size in file\n");

    Verbose(1, "nSamples: %d  parm: %ho\n", nSamples, parmKind);
    mNSamples = nSamples;
    mMapData = (float*)data;

    mBeginFrame = 0;
    mEndFrame = 0;
    if (iBeginTime)
        mBeginFrame = FrameIndex(iBeginTime);
    if (iEndTime)
        mEndFrame = FrameIndex(iEndTime);
    Verbose(1, "Begin frame %ld  End frame %ld\n", mBeginFrame, mEndFrame);
}

/**
 * The Fetch call is necessary to byte swap the data
 */
int Tracter::HTKSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    iIndex += mBeginFrame;

    int i;
    int offset = iOutputArea.offset;
    for (i=0; i<iOutputArea.Length(); i++)
    {
        if (iIndex >= mNSamples)
            break;
        if (mEndFrame && (iIndex > mEndFrame)) // EndFrame is inclusive
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
