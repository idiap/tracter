/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "VADGate.h"

Tracter::PluginObject* Tracter::VADGate::GetInput(int iInput)
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

Tracter::VADGate::VADGate(
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
    mSilenceConfirmed = -1;
    mIndexZero = 0;

    mEnabled = GetEnv("Enable", 1);
    mOnline = GetEnv("Online", 0);
}

/**
 * Catch reset.  Whether to pass upstream is an option.  In an online
 * mode, it shouldn't be passed on, but when the input is a sequence
 * of files it should be.
 */
void Tracter::VADGate::Reset(bool iPropagate)
{
    mSpeechTriggered = -1;
    mSpeechConfirmed = -1;
    CachedPlugin<float>::Reset(!mOnline);  // Propagate if not online
}

bool Tracter::VADGate::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);

    // iIndex is from the downstream point of view.  Reality could be ahead.
    iIndex += mIndexZero;

    // This will update iIndex to mSpeechTriggered 
    if (mEnabled && !gate(iIndex))
    {
        Verbose(1, "gate returned: index %ld silConf %ld\n",
                iIndex, mSilenceConfirmed);
        if (mOnline && (mSilenceConfirmed <= iIndex))
            throw Exception("iIndex ahead of silence");
        assert(
            (mSilenceConfirmed < 0) ||   /* Failed to find silence */
            (mSilenceConfirmed > iIndex) /* Succeeded */
        );
        mIndexZero = mSilenceConfirmed;
        mSpeechTriggered = -1;
        mSpeechConfirmed = -1;
        return false;
    }

    // Copy input to output
    CacheArea inputArea;
    if (mInput->Read(inputArea, iIndex) == 0)
        if (mOnline)
            throw Tracter::Exception("Unexpected out of input data");
        else
            return false;

    float* input = mInput->GetPointer(inputArea.offset);
    float* cache = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
        cache[i] = input[i];

    return true;
}


bool Tracter::VADGate::gate(IndexType& iIndex)
{
    if ((mSpeechTriggered < 0) && !confirmSpeech(iIndex))
    {
        // Failed to find any speech
        return false;
    }
    iIndex += mSpeechTriggered - mIndexZero;

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

bool Tracter::VADGate::readVADState(IndexType iIndex)
{
    CacheArea vadArea;
    if (mVADInput->Read(vadArea, iIndex) == 0)
        return false;
    VADState* state = mVADInput->GetPointer(vadArea.offset);
    mState = *state;
    return true;
}

bool Tracter::VADGate::confirmSpeech(IndexType iIndex)
{
    Verbose(1, "confirmSpeech entered\n");
    IndexType index = iIndex - 1;
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

    Verbose(1, "confirmSpeech: Frame %ld confirmed at %ld\n",
            mSpeechTriggered, mSpeechConfirmed);
    return true;
}

bool Tracter::VADGate::reconfirmSpeech(IndexType iIndex)
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

    assert(mState == SILENCE_CONFIRMED);
    mSilenceConfirmed = iIndex;
    Verbose(1, "reconfirmSpeech: Silence confirmed at %ld\n",
            mSilenceConfirmed);

    return false;
}
