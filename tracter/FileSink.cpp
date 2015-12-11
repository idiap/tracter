/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <string>
#include "FileSink.h"

Tracter::FileSink::FileSink(
    Component<float>* iInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mFeatureIndice = -1;
    mInput = iInput;
    Connect(iInput);
    mFrame.size = mInput->Frame().size;
    if (mFrame.size == 0)
        mFrame.size = 1;
    Initialise();
    Reset();

    mFile = 0;

    if (mByteOrder.WrongEndian())
        mTemp.resize(mFrame.size);
    mMaxSize = GetEnv("MaxSize", 0);
}

/**
 * Opens the given file and sucks data into it.
 */
void Tracter::FileSink::Open(const char* iFile)
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
        float* f = mInput->GetPointer(cache.offset);
        if (mByteOrder.WrongEndian())
        {
            for (int i=0; i<mFrame.size; i++)
                mTemp[i] = f[i];
            f = &mTemp[0];
            mByteOrder.Swap(f, sizeof(float), mFrame.size);
        }
        // All features are outputed
        if (mFeatureIndice < 0) {
            if (fwrite(f, sizeof(float), mFrame.size, mFile) != (size_t)mFrame.size)
                throw Exception("FileSink: Failed to write to file %s", iFile);
        } else {                
            assert(mFeatureIndice < mFrame.size);
            if (fprintf(mFile, "%f\n", f[mFeatureIndice]) < 0)
                throw Exception("FileSink: Failed to write to file %s", iFile);
        }
        if ((mMaxSize > 0) && (index >= mMaxSize))
            break;
    }

    if (fclose(mFile) != 0)
        throw Exception("FileSink: Could not close file %s", iFile);
    mFile = 0;
}
