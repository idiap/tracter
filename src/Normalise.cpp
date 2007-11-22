/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Normalise.h"

Normalise::Normalise(
    Plugin<short>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, short>(iInput)
{
    mObjectName = iObjectName;
    Endian endian = ENDIAN_NATIVE;
    const char* env = GetEnv("Endian", "NATIVE");
    if (env)
    {
        if (strcmp(env, "BIG") == 0)
            endian = ENDIAN_BIG;
        else if (strcmp(env, "LITTLE") == 0)
            endian = ENDIAN_LITTLE;
        else if (strcmp(env, "NATIVE") == 0)
            endian = ENDIAN_NATIVE;
        else
        {
            printf("Normalise: Unknown byte order: %s\n", env);
            exit(EXIT_FAILURE);
        }
    }
    mByteOrder.SetSource(endian);
}

void Normalise::MinSize(int iSize, int iReadAhead)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    PluginObject::MinSize(iSize, iReadAhead);

    // We expect the input buffer to be at least the size of each request
    assert(mInput);
    PluginObject::MinSize(mInput, iSize, 0);
}

int Normalise::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    int lenGot = mInput->Read(inputArea, iIndex, iOutputArea.Length());
    short* input = mInput->GetPointer();
    float* output = GetPointer();

    int rOffset = inputArea.offset;
    int wOffset = iOutputArea.offset;
    for (int i=0; i<lenGot; i++)
    {
        if (i == inputArea.len[0])
            rOffset = 0;
        if (i == iOutputArea.len[0])
            wOffset = 0;

        if (mByteOrder.WrongEndian())
        {        
            short s = input[rOffset++];
            mByteOrder.Swap(&s, 2, 1);
            output[wOffset++] = (float)s / 32768.0f;
        }
        else
            output[wOffset++] = (float)input[rOffset++] / 32768.0f;
    }

    return lenGot;
}
