/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "FileSink.h"

Tracter::FileSink::FileSink(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(iInput);
    mFrame.size = mInput->frame().size;
    if (mFrame.size == 0)
        mFrame.size = 1;
    initialise();
    reset();

    mFile = 0;

    if (mByteOrder.WrongEndian())
        mTemp.resize(mFrame.size);
    mMaxSize = config("MaxSize", 0);
}

/**
 * Opens the given file and sucks data into it.
 */
void Tracter::FileSink::open(const char* iFile)
{
    assert(iFile);
    assert(!mFile);

    mFile = fopen(iFile, "w");
    if (!mFile)
        throw Exception("FileSink: Failed to open file %s", iFile);

    /* Processing loop */
    int index = 0;
    CacheArea cache;
    while (mInput->Read(cache, index++))
    {
        float* f = mInput->getPointer(cache.offset);
        if (mByteOrder.WrongEndian())
        {
            for (int i=0; i<mFrame.size; i++)
                mTemp[i] = f[i];
            f = &mTemp[0];
            mByteOrder.Swap(f, sizeof(float), mFrame.size);
        }
        if (fwrite(f, sizeof(float), mFrame.size, mFile) != (size_t)mFrame.size)
            throw Exception("FileSink: Failed to write to file %s", iFile);
        if ((mMaxSize > 0) && (index >= mMaxSize))
            break;
    }

    if (fclose(mFile) != 0)
        throw Exception("FileSink: Could not close file %s", iFile);
    mFile = 0;
}
