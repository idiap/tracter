/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "HTKSink.h"

HTKSink::HTKSink(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnarySink<float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = mInput->GetArraySize();
    MinSize(iInput, 1);        

    mFile = 0;
    mByteOrder.SetTarget(ENDIAN_BIG);
    if (mByteOrder.WrongEndian())
        mTemp.resize(mArraySize);

    /* Initial header values */
    float period = mSamplePeriod / mSampleFreq;
    mNSamples = 0;
    mSampPeriod = (int)(period * 1e7f + 0.5);
    mSampSize = mArraySize * sizeof(float);
    mParmKind  = 6;      // MFCC
    mParmKind |= 000100; // _E
}

void HTKSink::WriteHeader(FILE* iFile)
{
    /* Copy header */
    int nSamples = mNSamples;
    int sampPeriod = mSampPeriod;
    short sampSize = mSampSize;
    short parmKind = mParmKind;

    /* Byte swap if necessary */
    if (mByteOrder.WrongEndian())
    {
        mByteOrder.Swap(&nSamples, 4, 1);
        mByteOrder.Swap(&sampPeriod, 4, 1);
        mByteOrder.Swap(&sampSize, 2, 1);
        mByteOrder.Swap(&parmKind, 2, 1);
    }

    /* Write */
    bool fail = false;
    fail |= (fwrite(&nSamples,   4, 1, iFile) != 1);
    fail |= (fwrite(&sampPeriod, 4, 1, iFile) != 1);
    fail |= (fwrite(&sampSize,   2, 1, iFile) != 1);
    fail |= (fwrite(&parmKind,   2, 1, iFile) != 1);
    if (fail)
    {
        printf("HTKSink: Failed to write HTK header\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Opens the given file and sucks data into it.
 */
void HTKSink::Open(const char* iFile)
{
    assert(iFile);
    assert(!mFile);

    mFile = fopen(iFile, "w");
    if (!mFile)
    {
        printf("HTKSink: Failed to open file %s\n", iFile);
        exit(EXIT_FAILURE);
    }

    WriteHeader(mFile);

    /* Processing loop */
    int index = 0;
    CacheArea cache;
    while (mInput->Read(cache, index++))
    {
        float* f = mInput->GetPointer(cache.offset);
        if (mByteOrder.WrongEndian())
        {
            for (int i=0; i<mArraySize; i++)
                mTemp[i] = f[i];
            f = &mTemp[0];
            mByteOrder.Swap(f, sizeof(float), mArraySize);
        }
        if (fwrite(f, sizeof(float), mArraySize, mFile) != (size_t)mArraySize)
        {
            printf("HTKSink: Failed to write to file %s\n", iFile);
            exit(EXIT_FAILURE);
        }
    }
    mNSamples = index - 1;
    rewind(mFile);
    WriteHeader(mFile);

    if (fclose(mFile) != 0)
    {
        printf("HTKSink: Could not close file %s\n", iFile);
        exit(EXIT_FAILURE);
    }
    mFile = 0;
}
