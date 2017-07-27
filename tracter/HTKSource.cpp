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
    objectName(iObjectName);
    mFrame.size = config("FrameSize", 39);
    mFrameRate = config("FrameRate", 100.0f);
    mFrame.period = config("FramePeriod", 1);

    mMapData = 0;
    mNSamples = 0;
    Endian endian = (Endian)config(cEndian, ENDIAN_BIG);
    mByteOrder.SetSource(endian);
}


/**
 * Maps the HTK parameter file and reads the header
 */
void Tracter::HTKSource::open(
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
    float objPeriod = mFrame.period / mFrameRate;
    float htkPeriod = sampPeriod * 1e-7;
    float subPeriod = htkPeriod - objPeriod;
    if ( (subPeriod >  1e-9) ||
         (subPeriod < -1e-9) )
        throw Exception("HTKSource:"
                        " sample period %f not equal to expected period %f",
                        objPeriod, htkPeriod);

    if (sampSize/4 != mFrame.size)
        throw Exception("HTKSource:"
                        " sample size %d/4 not equal to expected size %d\n",
                        sampSize, mFrame.size);

    if (nSamples * sampSize + 12 > mMap.Size())
        throw Exception(
            "HTKSource:"
            " data size %d in header not equal to size in file %d\n",
            nSamples * sampSize + 12, mMap.Size()
        );

    verbose(1, "nSamples: %d  parm: %ho\n", nSamples, parmKind);
    mNSamples = nSamples;
    mMapData = (float*)data;

    mBeginFrame = 0;
    mEndFrame = -1;
    if (iBeginTime >= 0)
        mBeginFrame = frameIndex(iBeginTime);
    if (iEndTime >= 0)
        mEndFrame = frameIndex(iEndTime);
    verbose(1, "Begin frame %ld  End frame %ld\n", mBeginFrame, mEndFrame);
}

/**
 * The Fetch call is necessary to byte swap the data
 */
Tracter::SizeType
Tracter::HTKSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    iIndex += mBeginFrame;

    SizeType i;
    SizeType offset = iOutputArea.offset;
    for (i=0; i<iOutputArea.length(); i++)
    {
        if (iIndex >= mNSamples)
            break;
        if ((mEndFrame >= 0) && (iIndex > mEndFrame)) // EndFrame is inclusive
            break;
        if (i == iOutputArea.len[0])
            offset = 0;
        float* cache = getPointer(offset);
        for (int j=0; j<mFrame.size; j++)
            cache[j] = mMapData[iIndex*mFrame.size + j];
        if (mByteOrder.WrongEndian())
            mByteOrder.Swap(cache, 4, mFrame.size);
        iIndex++;
        offset++;
    }
    return i;
}
