/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>
#include <cstdio>

#include "ByteOrder.h"

/** Determine native byte order and initialise source and target ordering */
Tracter::ByteOrder::ByteOrder()
{
    /* A short and two chars in the same location */
    assert(sizeof(short) == 2);
    assert(sizeof(char)  == 1);
    union {
        char c[2];
        short s;
    } endianTest;

    /* Find the native endian */
    endianTest.s = 1;
    if (endianTest.c[0] == 1)
    {
        assert(endianTest.c[1] == 0);
        mNative = ENDIAN_LITTLE;
    }
    else
    {
        assert(endianTest.c[1] == 1);
        mNative = ENDIAN_BIG;
    }

    mSource = mNative;
    mTarget = mNative;
}


/**
 * Swaps byte order unconditionally for an array of even sized
 * elements.
 */
void Tracter::ByteOrder::Swap(
    void* iData,      ///< Pointer to data to swap
    size_t iDataSize, ///< Size of each datum
    int iDataCount    ///< Number of data
)
{
    assert(iDataCount >= 0);
    char* data = (char*)iData;
    int halfSize = iDataSize/2;
    assert(halfSize*2 == (int)iDataSize);  // i.e. iDataSize is even
    for(int i=0; i<iDataCount; i++)
    {
        for(int j=0; j<halfSize; j++)
        {
            int jj = iDataSize-j-1;
            char tmp = data[j];
            data[j] = data[jj];
            data[jj] = tmp;
        }
        data += iDataSize;
    }
}
