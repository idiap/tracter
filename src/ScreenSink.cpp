/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ScreenSink.h"

ScreenSink::ScreenSink(
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

    mMaxSize = GetEnv("MaxSize", 0);
}

/**
 * Suck data onto screen.
 */
void ScreenSink::Open()
{

    /* Processing loop */
    int index = 0;
    CacheArea cache;
    while (mInput->Read(cache, index))
    {
        float* f = mInput->GetPointer(cache.offset);
        printf("%d: ", index++ );
        for (int i = 0 ; i < mArraySize ; i++ ){
			printf( "%.3f ",f[i]);
        }
        printf("\n");
        if ((mMaxSize > 0) && (index >= mMaxSize))
            break;
    }
}
