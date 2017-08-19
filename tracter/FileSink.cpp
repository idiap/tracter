/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "FileSink.h"

Tracter::FileSink::FileSink(Component<float>* iInput, const char* iObjectName)
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
    mBinary = config("Binary", "0");

    if (mByteOrder.wrongEndian())
        mTemp.resize(mFrame.size);
    mMaxSize = config("MaxSize", 0);
}

/**
 * Opens the given file and sucks data into it.  File defaults stdout.
 */
void Tracter::FileSink::open(const char* iFile)
{
    if (iFile)
    {
        mFile = fopen(iFile, "w");
        if (!mFile)
            throw Exception("FileSink: Failed to open file %s", iFile);
    }
    else
        mFile = stdout;

    /* Processing loop */
    int index = 0;
    CacheArea cache;
    while (mInput->read(cache, index++))
    {
        float* f = mInput->getPointer(cache.offset);
        if (mBinary)
        {
            if (mByteOrder.wrongEndian())
            {
                for (int i=0; i<mFrame.size; i++)
                    mTemp[i] = f[i];
                f = &mTemp[0];
                mByteOrder.swap(f, sizeof(float), mFrame.size);
            }
            if (fwrite(f, sizeof(float), mFrame.size, mFile)
                != (size_t)mFrame.size)
                throw Exception("FileSink: Failed to write to file %s", iFile);
        }
        else
        {
            printf("%d: ", index++ );
            for (int i = 0 ; i < mFrame.size ; i++ )
                printf( "%.3f ",f[i]);
            printf("\n");
        }
        if ((mMaxSize > 0) && (index >= mMaxSize))
            break;
    }

    if (mFile != stdout)
        if (fclose(mFile) != 0)
            throw Exception("FileSink: Could not close file %s", iFile);
    mFile = 0;
}
