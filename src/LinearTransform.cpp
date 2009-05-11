/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "LinearTransform.h"

Tracter::LinearTransform::LinearTransform(
    Plugin<float>* iInput, const char* iObjectName
)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    MinSize(mInput, 1);

    const char* file = GetEnv("XFormFile", (const char*)0);
    mArraySize = LoadXForm(file);
    if (mArraySize * mInput->GetArraySize() != (int)mMatrix.size())
        throw Exception("input dimension %d incompatible with matrix cols %d",
                        mInput->GetArraySize(), (int)mMatrix.size()/mArraySize);
}

bool Tracter::LinearTransform::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(mMatrix.size() > 0);

    // Read the input frame
    const float* p = mInput->UnaryRead(iIndex);
    if (!p)
        return false;

    // Output
    float* cache = GetPointer(iOffset);

    /* Multiply, with due disregard to cache and BLAS */
    for (int r=0; r<mArraySize; r++)
    {
        cache[r] = 0.0f;
        for (int c=0; c<mInput->GetArraySize(); c++)
            cache[r] += mMatrix[r*mArraySize + c] * p[c];
    }

    return true;
}

int Tracter::LinearTransform::LoadXForm(const char* iFileName)
{
    if (!iFileName)
        throw Exception("Null file name");

    FILE* fp = fopen(iFileName, "r");
    if (!fp)
        throw Exception("Failed to open file %s", iFileName);

    /* Read until the <XForm> tag */
    char tmpStr[1024] = "";
    do
    {
        if (fscanf(fp, "%s", tmpStr) != 1)
            throw Exception("Failed to read <XForm> token");
    }
    while (strncasecmp(tmpStr, "<Xform>", 1024));

    /* Read the row and column dimensions */
    int nRow, nCol;
    if (fscanf(fp, "%d %d", &nRow, &nCol) != 2)
        throw Exception("Failed to read size tokens");

    /* And finally the matrix itself */
    int size = nRow * nCol;
    mMatrix.resize(size);
    for (int i=0; i<size; i++)
        if (fscanf(fp, "%f", &mMatrix[i]) != 1)
            throw Exception("failed to read element %d", i);

    /* Done */
    fclose(fp);
    return nRow;
}
