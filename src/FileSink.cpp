/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "FileSink.h"

FileSink::FileSink(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnarySink<float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = mInput->GetArraySize();
    if (mArraySize == 0)
        mArraySize = 1;
    MinSize(iInput, 1);
    Initialise();
    Reset();

    mFile = 0;
    mByteOrder.SetTarget(ENDIAN_BIG);
    if (mByteOrder.WrongEndian())
        mTemp.resize(mArraySize);
    mMaxSize = GetEnv("MaxSize", 0);
}

/**
 * Opens the given file and sucks data into it.
 */
void FileSink::Open(const char* iFile)
{
    assert(iFile);
    assert(!mFile);

    mFile = fopen(iFile, "w");
    if (!mFile)
    {
        printf("FileSink: Failed to open file %s\n", iFile);
        exit(EXIT_FAILURE);
    }

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
            printf("FileSink: Failed to write to file %s\n", iFile);
            exit(EXIT_FAILURE);
        }
        if ((mMaxSize > 0) && (index >= mMaxSize))
            break;
    }

    if (fclose(mFile) != 0)
    {
        printf("FileSink: Could not close file %s\n", iFile);
        exit(EXIT_FAILURE);
    }
    mFile = 0;
}
