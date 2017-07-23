/*
 * Copyright 2015 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cmath>

#include "BoolToFloat.h"

/**
 * Constructor that connect component with input.
 */
Tracter::BoolToFloat::BoolToFloat(Component<BoolType>* iInput, const char* iObjectName) 
{
    objectName(iObjectName);
    mInput = iInput;
    Connect(mInput);
    mFrame.size = iInput->Frame().size;
    assert(mFrame.size >= 0);
}

/**
 * Fetch value of input at indice 'iIndex' and put it into
 * 'oData' as a float.
 *
 * return true if some data is fetched otherwise false when
 *        end of stream is reached.
 */
bool Tracter::BoolToFloat::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    // Read the input frame at indice 'iIndex'
    const BoolType* open = mInput->UnaryRead(iIndex);

    if (!open) {
        Verbose(2, "BoolToFloat: End Of Data at %ld\n", iIndex);
        return false;
    }

    bool mBool = *open;

    if (mBool == true)
    	oData[0] = (float) 1;
    else
    	oData[0] = (float) 0;
    
    return true;
}
