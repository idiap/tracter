/*
 * Copyright 2016 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, January 2016
 */

#include "CochlearFilter.h"

namespace Tracter
{
    enum {
        HOLDSWORTH,
        LYON,
        CASCADE
    };
    const StringEnum cFilter[] = {
        {"Holdsworth", HOLDSWORTH},
        {"Lyon",       LYON},
        {"Cascade",    CASCADE},
        {0,            -1}
    };
}

using namespace Tracter;

CochlearFilter::CochlearFilter(
    Component<float>* iInput, const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(mInput);
    mIndex = -1;
    mFrame.size = config("Size", 23);
    float loHertz = config("LoHertz", 64.0f);
    float hiHertz = config("HiHertz", 4000.0f);
    int type = config(cFilter, LYON);
    switch (type)
    {
    case HOLDSWORTH:
        mCochlea = new ssp::Holdsworth(
            loHertz, hiHertz, mFrame.size, 1.0f / frameRate()
        );
        break;
    case LYON:
        mCochlea = new ssp::Lyon(
            loHertz, hiHertz, mFrame.size, 1.0f / frameRate()
        );
        break;
    case CASCADE:
        mCochlea = new ssp::Cascade(
            loHertz, hiHertz, mFrame.size, 1.0f / frameRate()
        );
        break;
    }
}

CochlearFilter::~CochlearFilter()
{
    if (mCochlea)
        delete mCochlea;
    mCochlea = 0;
}

void Tracter::CochlearFilter::minSize(
    SizeType iSize, SizeType iReadBehind, SizeType iReadAhead
)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    ComponentBase::minSize(iSize, iReadBehind, iReadAhead);

    // We expect the input buffer to be at least the size of each request
    assert(mInput);
    ComponentBase::minSize(mInput, iSize, 0, 0);
}

void CochlearFilter::reset(bool iPropagate)
{
    mCochlea->reset();
    mIndex = -1;
    CachedComponent<float>::reset(iPropagate);
}

SizeType CochlearFilter::contiguousFetch(
    IndexType iIndex, SizeType iLength, SizeType iOffset
)
{
    // The SSP Cochlea class need samples in order.  So, if set, mIndex is the
    // next expected value of iIndex.
    if (mIndex < 0)
        mIndex = iIndex;
    else
        assert(iIndex == mIndex);

    float* op = getPointer(iOffset);
    int nRead = 0;
    while (nRead < iLength)
    {
        SizeType n = iLength-nRead;
        const float* ip = mInput->contiguousRead(mIndex, n);
        if (!ip)
            return nRead;
        for (int i=0; i<n; i++)
        {
            (*mCochlea)(ip[i], op);
            op += mFrame.size;
        }
        nRead += n;
        mIndex += n;
    }

    // If the loop returns then all data were processed
    return iLength;
}
