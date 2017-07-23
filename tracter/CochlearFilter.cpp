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
    Connect(mInput);
    mIndex = -1;
    mFrame.size = GetEnv("Size", 23);
    float loHertz = GetEnv("LoHertz", 64.0f);
    float hiHertz = GetEnv("HiHertz", 4000.0f);
    int type = GetEnv(cFilter, LYON);
    switch (type)
    {
    case HOLDSWORTH:
        mCochlea = new ssp::Holdsworth(
            loHertz, hiHertz, mFrame.size, 1.0f / FrameRate()
        );
        break;
    case LYON:
        mCochlea = new ssp::Lyon(
            loHertz, hiHertz, mFrame.size, 1.0f / FrameRate()
        );
        break;
    case CASCADE:
        mCochlea = new ssp::Cascade(
            loHertz, hiHertz, mFrame.size, 1.0f / FrameRate()
        );
        break;
    }
}

CochlearFilter::~CochlearFilter() throw()
{
    if (mCochlea)
        delete mCochlea;
    mCochlea = 0;
}

void Tracter::CochlearFilter::MinSize(
    SizeType iSize, SizeType iReadBehind, SizeType iReadAhead
)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    ComponentBase::MinSize(iSize, iReadBehind, iReadAhead);

    // We expect the input buffer to be at least the size of each request
    assert(mInput);
    ComponentBase::MinSize(mInput, iSize, 0, 0);
}

void CochlearFilter::Reset(bool iPropagate)
{
    mCochlea->reset();
    mIndex = -1;
    CachedComponent<float>::Reset(iPropagate);
}

SizeType CochlearFilter::ContiguousFetch(
    IndexType iIndex, SizeType iLength, SizeType iOffset
)
{
    // The SSP Cochlea class need samples in order.  So, if set, mIndex is the
    // next expected value of iIndex.
    if (mIndex < 0)
        mIndex = iIndex;
    else
        assert(iIndex == mIndex);

    float* op = GetPointer(iOffset);
    int nRead = 0;
    while (nRead < iLength)
    {
        SizeType n = iLength-nRead;
        const float* ip = mInput->ContiguousRead(mIndex, n);
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
