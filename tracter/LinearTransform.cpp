/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cstring>

#include "LinearTransform.h"

Tracter::LinearTransform::LinearTransform(
    Component<float>* iInput, const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(mInput);

    const char* file = GetEnv("XFormFile", (const char*)0);
    mFrame.size = LoadXForm(file);
    if (mFrame.size * mInput->Frame().size != (int)mMatrix.size())
        throw Exception("input dimension %d incompatible with matrix cols %d",
                        mInput->Frame().size, (int)mMatrix.size()/mFrame.size);
}

bool Tracter::LinearTransform::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(mMatrix.size() > 0);

    // Read the input frame
    const float* p = mInput->UnaryRead(iIndex);
    if (!p)
        return false;

    /* Multiply, with due disregard to cache and BLAS */
    int nCols = mInput->Frame().size;
    for (int r=0; r<mFrame.size; r++)
    {
        oData[r] = 0.0f;
        for (int c=0; c<nCols; c++)
            oData[r] += mMatrix[r*nCols + c] * p[c];
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
