/*
 * Copyright 2016 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, January 2016
 */

#include "CochlearFrame.h"

namespace Tracter
{
    enum {
        METHOD_MAX,
        METHOD_MAG,
        METHOD_ENERGY
    };

    const StringEnum cMethod[] = {
        {"Max",       METHOD_MAX},
        {"Mag",       METHOD_MAG},
        {"Energy",    METHOD_ENERGY},
        {0,          -1}
    };
}

using namespace Tracter;

CochlearFrame::CochlearFrame(
    Component<float>* iInput, const char* iObjectName
)
{
    mObjectName = iObjectName;
    mInput = iInput;
    mSize = GetEnv("Size", 256);
    mFrame.size = mInput->Frame().size;
    mFrame.period = GetEnv("Period", 80);

    // Framers look ahead, not back
    Connect(mInput, mSize, mSize-1);

    mMethod = GetEnv(cMethod, METHOD_ENERGY);
    mWindow = 0;
    if (GetEnv("Window", 0))
        mWindow = new Window(mObjectName, mSize);

    assert(mSize > 0);
    assert(mFrame.size > 0);
    assert(mFrame.period > 0);
}

CochlearFrame::~CochlearFrame()
{
    if (mWindow)
        delete mWindow;
    mWindow = 0;
}

bool CochlearFrame::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Zero the output vector as it's workspace too
    for (int j=0; j<mFrame.size; j++)
        oData[j] = 0.0f;

    // Loop over mSize input frames
    IndexType readIndex = iIndex * mFrame.period;
    int nRead = 0;
    while (nRead < mSize)
    {
        SizeType n = mSize-nRead;
        const float* ip = mInput->ContiguousRead(readIndex, n);
        if (!ip)
            return false;
        for (int i=0; i<n; i++)
        {
            for (int j=0; j<mFrame.size; j++)
            {
                float f = ip[j];
                if (mWindow)
                    f *= mWindow->at(nRead+i);
                switch (mMethod)
                {
                case METHOD_MAX:
                    oData[j] = std::max(f*f, oData[j]);
                    break;
                case METHOD_MAG:
                    oData[j] += std::abs(f);
                    break;
                case METHOD_ENERGY:
                    oData[j] += f*f;
                    break;
                }
            }
            ip += mFrame.size;
        }
        nRead += n;
        readIndex += n;
    }

    // Done
    return true;
}
