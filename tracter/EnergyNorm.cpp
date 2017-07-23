/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "EnergyNorm.h"
#include "math.h"

Tracter::EnergyNorm::EnergyNorm(
    Component<float>* iInput1,
    /*Component<float>* iInput2,*/
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput1 = iInput1;
    //mInput2 = iInput2;
    Connect(iInput1);
    //Connect(iInput2);

    mFrame.size = iInput1->Frame().size;
    assert(mFrame.size == 1);

    m_maxE = -2.5;
}

bool Tracter::EnergyNorm::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);
    CacheArea inputArea;

    // Start with the second input, likely to be a cepstral mean
    //if (mInput2->Read(inputArea, iIndex) == 0)
    //    return false;
    //float *p2 = mInput2->GetPointer(inputArea.offset);

    // Now the first input
    if (mInput1->Read(inputArea, iIndex) == 0)
        return false;
    float *p1 = mInput1->GetPointer(inputArea.offset);

    // printf("%f\n",*p1);
    if (*p1 > m_maxE)
      m_maxE = *p1;

    // Do the normalisation
    *oData = *p1 - m_maxE + 1.0;

    return true;
}
