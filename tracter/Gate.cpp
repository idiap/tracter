/*
 * Copyright 2011 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2011 (Based on VADGate.cpp)
 */

#include "Gate.h"

Tracter::Gate::Gate(
    Component<float>* iInput,
    Component<BoolType>* iControlInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mFrame.size = iInput->frame().size;

    connect(iInput);
    connect(iControlInput);

    mInput = iInput;
    mControlInput = iControlInput;
    mOpenedIndex = -1;
    mClosedIndex = -1;
    mIndexZero = 0;
    mRemoved = 0;
    mOpen = false;
    mUpstreamEndOfData = false;

    mEnabled = config("Enable", 1);
    mSegmenting = config("Segmenting", 0);
    mConcatenate = config("Concatenate", 0);
}

/**
 * Catch reset.  Whether to pass upstream is an option.  In an online
 * mode, it shouldn't be passed on, but when the input is a sequence
 * of files it should be.
 */
void Tracter::Gate::reset(bool iPropagate)
{
    verbose(2, "Resetting\n");
    if (mSegmenting)
    {
        if (mClosedIndex >= 0)
            mIndexZero = mClosedIndex;
    }
    else
        mIndexZero = 0;
    mOpen = false;
    mOpenedIndex = -1;
    mClosedIndex = -1;
    mRemoved = 0;

    // Propagate reset upstream under these conditions
    CachedComponent<float>::reset(
        mUpstreamEndOfData ||  // Always after EOD
        !mSegmenting ||        // If not segmenting
        !mEnabled              // If disabled
    );
    mUpstreamEndOfData = false;
}

bool Tracter::Gate::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);

    // gate() passes by reference and will update iIndex to the
    // upstream point of view.
    if (mEnabled && !gate(iIndex))
    {
        verbose(2, "gate() returned at Index %ld, ClosedIndex %ld\n",
                iIndex, mClosedIndex);

        // If all went well, mClosedIndex should be equal to iIndex
        if ( mSegmenting &&
             (mClosedIndex >= 0) &&
             (mClosedIndex < iIndex) )
            throw Exception("iIndex ahead of silence");
        assert(
            (mClosedIndex < 0) ||    /* Failed to find silence */
            (mClosedIndex >= iIndex) /* Succeeded */
        );

        // Must leave mIndexZero alone until reset so the downstream
        // components can query time properly

        return false;
    }

    // Copy input to output
    const float* ip = mInput->unaryRead(iIndex);
    if (!ip)
        return false;
    for (int i=0; i<mFrame.size; i++)
        oData[i] = ip[i];

    return true;
}

/**
 * Advance the index to the next valid frame depending on the control
 * input.  iIndex is from the downstream point of view, but is updated
 * to represent the upstream point of view, which could be larger
 * representing skipped over data.
 *
 * Returns true if data should be output for the given index.
 */
bool Tracter::Gate::gate(IndexType& iIndex)
{
    assert(iIndex >= 0);
    assert(mIndexZero >= 0);

    // If the gate has already been closed, it must be reset before
    // reopening
    if (mClosedIndex >= 0)
        return false;

    // iIndex is from the downstream point of view.  Reality could be
    // ahead.
    iIndex += mIndexZero;

    if ((mOpenedIndex < 0) && !openGate(iIndex))
    {
        // Failed to find any control
        return false;
    }

    // Gate is open.  Set the state based on the next frame
    assert(mOpen == true);
    assert(mOpenedIndex >= 0);
    iIndex += mOpenedIndex - mRemoved - mIndexZero;
    if (!readControl(iIndex))
    {
        mClosedIndex = iIndex + 1;
        return false;
    }

    // If it closed, check if concatenating is enabled and try to
    // reopen
    if (mOpen == false)
    {
        //assert(mClosedIndex >= mOpenedIndex);
        if (mConcatenate)
        {
            mRemoved += mClosedIndex - mOpenedIndex;
            if (!openGate(mClosedIndex))
                return false;
            if (mOpen)
                mClosedIndex = -1; // i.e., it's not closed anymore
        }
        else
        {
            mClosedIndex = iIndex;
            return false;
        }
    }

    return true;
}

/**
 * Reads the control state for the given index into mOpen and returns
 * true.  Returns false if EOD.
 */
bool Tracter::Gate::readControl(IndexType iIndex)
{
    assert(iIndex >= 0);

    const BoolType* open = mControlInput->unaryRead(iIndex);
    if (!open)
    {
        verbose(2, "readControl: End Of Data at %ld\n", iIndex);
        mUpstreamEndOfData = true;
        return false;
    }
    mOpen = *open;
    return true;
}

/**
 * Assuming the gate is closed, typically at the beginning of a file,
 * advance to an index where the gate is open.  This will set
 * mOpenedIndex, then return true.  Returns false if no valid control
 * could be found.
 */
bool Tracter::Gate::openGate(IndexType iIndex)
{
    verbose(2, "Attempting to open gate\n");
    assert(iIndex >= 0);
    assert(mOpen == false);

    IndexType index = iIndex - 1; // Pre-incremented immediately
    do
    {
        if (!readControl(++index))
            return false;
    }
    while (mOpen == false);

    assert(mOpen == true);
    mOpenedIndex = index;
    verbose(2, "openGate: opened at %ld\n", mOpenedIndex);
    return true;
}
