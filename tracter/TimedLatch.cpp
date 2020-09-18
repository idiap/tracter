/*
 * Copyright 2011 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2011
 */

#include "TimedLatch.h"

Tracter::TimedLatch::TimedLatch(
    Component<BoolType>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mFrame.size = iInput->frame().size;
    mInput = iInput;
    connect(iInput);

    // Minimum times
    float confirmTrueTime = config("ConfirmTrueTime", 0.3f);
    float confirmFalseTime = config("ConfirmFalseTime", 0.3f);
    mConfirmTrueTime = secondsToFrames(confirmTrueTime);
    mConfirmFalseTime = secondsToFrames(confirmFalseTime);

    int max = std::max(mConfirmTrueTime, mConfirmFalseTime);
    minSize(iInput, max, max-1);

    mIndex = -1;
    mState = false;
}

/**
 * Catch reset.
 */
void Tracter::TimedLatch::reset(bool iPropagate)
{
    mIndex = -1;
    mState = false;
    CachedComponent<BoolType>::reset(iPropagate);
}

bool Tracter::TimedLatch::unaryFetch(IndexType iIndex, BoolType* oData)
{
    assert(iIndex >= 0);
    assert(oData);
    assert((mIndex < 0) || (iIndex = mIndex + 1));

    const BoolType* state = mInput->unaryRead(iIndex);
    if (!state)
        return false;

    /*
     * This is a bit inefficient because it reads ahead for each
     * successive frame that suggests a false change of state.  It
     * would be much better if it were possible to write ahead too.
     */
    if (mState)
    {
        if (*state)
        {
            // true -> true
            mState = true;
        }
        else
        {
            // true -> false?  Assume yes, then check for no.
            mState = false;
            for (int i=1; i<mConfirmFalseTime; i++)
            {
                state = mInput->unaryRead(iIndex+i);
                if (!state || *state)
                {
                    // Either EOD, or data saying true, so revert to true
                    mState = true;
                    break;
                }
            }
        }
    }
    else
    {
        if (*state)
        {
            // false -> true?  Assume yes, then check for no.
            mState = true;
            for (int i=1; i<mConfirmTrueTime; i++)
            {
                state = mInput->unaryRead(iIndex+i);
                if (!state || !*state)
                {
                    // Either EOD, or data saying false, so revert to false
                    mState = false;
                    break;
                }
            }
        }
        else
        {
            // false -> false
            mState = false;
        }
    }

    // The current output just drops out in mState
    *oData = mState;
    mIndex = iIndex;
    return true;
}
