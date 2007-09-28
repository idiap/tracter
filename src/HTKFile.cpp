#include <stdlib.h>
#include "HTKFile.h"

/**
 * Swaps byte order for an array of even sized elements
 */
void ByteSwap(char* ioData, size_t iDataSize, size_t iDataCount)
{
    size_t halfSize = iDataSize/2;
    assert(2*halfSize == iDataSize);  // i.e. iDataSize is even
    for(size_t i=0; i<iDataCount; i++)
    {
        for(size_t j=0; j<halfSize; j++)
        {
            size_t jj = iDataSize-j-1;
            char tmp = ioData[j];
            ioData[j] = ioData[jj];
            ioData[jj] = tmp;
        }
        ioData += iDataSize;
    }
}

HTKFile::HTKFile(int iArraySize)
    : CachedPlugin<float>(iArraySize)
{
    assert(iArraySize > 0);
    mArraySize = iArraySize;
    mMapData = 0;
    mNSamples = 0;
}


/**
 * Maps the HTK parameter file and reads the header
 */
void HTKFile::Map(const char* iFileName)
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
    ByteSwap((char*)&nSamples, 4, 1);
    data += 4;

    sampPeriod = *(int*)data;
    ByteSwap((char*)&sampPeriod, 4, 1);
    data += 4;

    sampSize = *(short*)data;
    ByteSwap((char*)&sampSize, 2, 1);
    data += 2;

    parmKind = *(short*)data;
    ByteSwap((char*)&parmKind, 2, 1);
    data += 2;

    if (sampSize/4 != mArraySize)
    {
        printf("HTKFile: sample size %d/4 not equal to expected size %d\n",
               sampSize, mArraySize);
        exit(EXIT_FAILURE);
    }

    if ((size_t)nSamples * sampSize + 12 != mMap.GetSize())
    {
        printf("HTKFile: data size in header not equal to size in file\n");
        exit(EXIT_FAILURE);
    }

    printf("nSamples: %d  parm: %ho\n", nSamples, parmKind);
    mNSamples = nSamples;
    mMapData = (float*)data;
}

/**
 * The process call is necessary to byte swap the data
 */
int HTKFile::Process(IndexType iIndex, CacheArea& iOutputArea)
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
        ByteSwap((char*)cache, 4, mArraySize);
        iIndex++;
        offset++;
    }
    return i;
}
