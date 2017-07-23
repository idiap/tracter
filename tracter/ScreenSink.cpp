/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ScreenSink.h"

Tracter::ScreenSink::ScreenSink(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    Connect(mInput);
    mFrame.size = mInput->Frame().size;
    if (mFrame.size == 0)
        mFrame.size = 1;
    Initialise();
    Reset();

    mMaxSize = config("MaxSize", 0);
}

/**
 * Suck data onto screen.
 */
void Tracter::ScreenSink::Open(const char* iFile)
{

    /* Processing loop */
    int index = 0;
    CacheArea cache;
    while (mInput->Read(cache, index))
    {
        float* f = mInput->GetPointer(cache.offset);
        printf("%d: ", index++ );
        for (int i = 0 ; i < mFrame.size ; i++ ){
            printf( "%.3f ",f[i]);
        }
        printf("\n");
        if ((mMaxSize > 0) && (index >= mMaxSize))
            break;
    }
}
