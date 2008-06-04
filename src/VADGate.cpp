/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "VADGate.h"

PluginObject* VADGate::GetInput(int iInput)
{
    // Enumerate the inputs
    switch (iInput)
    {
        case 0:
            return mInput;
        case 1:
            return mVADInput;
        default:
            assert(0);
    }

    // Should never get here
    return 0;
}

VADGate::VADGate(
    Plugin<float>* iInput,
    Plugin<VADState>* iVADInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mArraySize = iInput->GetArraySize();

    Connect(iInput);
    Connect(iVADInput);
    MinSize(iInput, 1);
    MinSize(iVADInput, 1);

    mInput = iInput;
    mVADInput = iVADInput;
    mSpeechTriggered = -1;
    mSpeechConfirmed = -1;

    mEnabled = GetEnv("Enable", 1);
}

bool VADGate::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);

    if (mEnabled && !gate(iIndex))
        return false;

    // Copy input to output
    CacheArea inputArea;
    if (mInput->Read(inputArea, iIndex) == 0)
        return false;
    float* input = mInput->GetPointer(inputArea.offset);
    float* cache = GetPointer(iOffset);
    //printf("Input: %e %e %e\n", input[0], input[1], input[2]);
    for (int i=0; i<mArraySize; i++)
        cache[i] = input[i];

    return true;
}


bool VADGate::gate(IndexType& iIndex)
{
    if ((mSpeechTriggered < 0) && !confirmSpeech())
    {
        // Failed to find any speech
        mSpeechTriggered = -1;
        return false;
    }
    iIndex += mSpeechTriggered;

    if ((iIndex > mSpeechConfirmed))
    {
        if (!readVADState(iIndex))
            return false;
        if ((mState == SILENCE_TRIGGERED) && !reconfirmSpeech(iIndex))
        {
            return false;
        }
    }

    return true;
}

bool VADGate::readVADState(IndexType iIndex)
{
    CacheArea vadArea;
    if (mVADInput->Read(vadArea, iIndex) == 0)
        return false;
    VADState* state = mVADInput->GetPointer(vadArea.offset);
    mState = *state;
    return true;
}

bool VADGate::confirmSpeech()
{
    IndexType index = -1;
    do
    {
        do
        {
            if (!readVADState(++index))
                return false;
        }
        while (mState == SILENCE_CONFIRMED);
        assert(mState == SPEECH_TRIGGERED);
        mSpeechTriggered = index;

        do
        {
            if (!readVADState(++index))
                return false;
        }
        while (mState == SPEECH_TRIGGERED);
    }
    while (mState != SPEECH_CONFIRMED);
    mSpeechConfirmed = index;

    if (Tracter::sVerbose > 0)
        printf("VADGate::confirmSpeech: Found at frame %ld\n",
               mSpeechTriggered);
    return true;
}

bool VADGate::reconfirmSpeech(IndexType iIndex)
{
    assert(mState == SILENCE_TRIGGERED);
    do
    {
        if (!readVADState(++iIndex))
            return false;
    }
    while (mState == SILENCE_TRIGGERED);

    if (mState == SPEECH_CONFIRMED)
    {
        mSpeechConfirmed = iIndex;
        return true;
    }
    return false;
}
