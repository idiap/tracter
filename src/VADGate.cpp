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
    VADStateMachine* iVADInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mArraySize = iInput->GetArraySize();

    Connect(iInput);
    Connect(iVADInput);
    MinSize(iInput, 1);

    int max = std::max(iVADInput->ConfirmSpeechTime(),
                       iVADInput->ConfirmSilenceTime());
    MinSize(iVADInput, max);

    mInput = iInput;
    mVADInput = iVADInput;
    mSpeechTriggered = -1;
    mSpeechConfirmed = -1;
    mSilenceConfirmed = -1;
    mIndexZero = 0;
    mSpeechRemoved = 0;
    mState = SILENCE_CONFIRMED;
    mUpstreamEndOfData = false;

    mEnabled = GetEnv("Enable", 1);
    mSegmenting = GetEnv("Segmenting", 0);
    mRemoveSilence = GetEnv("RemoveSilence", 0);
}

/**
 * Catch reset.  Whether to pass upstream is an option.  In an online
 * mode, it shouldn't be passed on, but when the input is a sequence
 * of files it should be.
 */
void Tracter::VADGate::Reset(bool iPropagate)
{
    mState = SILENCE_CONFIRMED;
    mSpeechTriggered = -1;
    mSpeechConfirmed = -1;
    mSilenceConfirmed = -1;
    mSpeechRemoved = 0;
    if (!mSegmenting)
    {
        mIndexZero = 0;
    }

    // Propagate if not segmenting.  Always propagate after EOD.
    CachedPlugin<float>::Reset(mUpstreamEndOfData || !mSegmenting);
    mUpstreamEndOfData = false;
}

bool Tracter::VADGate::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);

    // gate() passes by reference and will update iIndex to the
    // upstream point of view.
    if (mEnabled && !gate(iIndex))
    {
        Verbose(1, "gate() returned at index %ld, silConf %ld\n",
                iIndex, mSilenceConfirmed);
        if ( mSegmenting &&
             (mSilenceConfirmed >= 0) &&
             (mSilenceConfirmed <= iIndex) )
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
        return false;

    float* input = mInput->GetPointer(inputArea.offset);
    float* cache = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
        cache[i] = input[i];

    return true;
}

/**
 * Returns true if speech should be output for the given index.
 * iIndex is from the downstream point of view, but is updated to
 * represent the upstream point of view, which could be larger
 * representing skipped over data.
 */
bool Tracter::VADGate::gate(IndexType& iIndex)
{
    assert(iIndex >= 0);
    assert(mIndexZero >= 0);

    // iIndex is from the downstream point of view.  Reality could be ahead.
    iIndex += mIndexZero;

    if ((mSpeechTriggered < 0) && !confirmSpeech(iIndex))
    {
        // Failed to find any speech
        return false;
    }

    assert(mState == SPEECH_CONFIRMED);
    assert(mSpeechTriggered >= 0);
    assert(mSpeechConfirmed >= 0);

    iIndex += mSpeechTriggered - mSpeechRemoved - mIndexZero;
    if ((iIndex > mSpeechConfirmed))
    {
        if (!readVADState(iIndex))
        {
            mSilenceConfirmed = iIndex + 1;
            return false;
        }
        if ((mState == SILENCE_TRIGGERED) && !reconfirmSpeech(iIndex))
        {
            assert(mSilenceConfirmed >= mSpeechTriggered);
            if (mRemoveSilence && (mState == SILENCE_CONFIRMED))
            {
                mSpeechRemoved += mSilenceConfirmed - mSpeechTriggered;
                if (!confirmSpeech(mSilenceConfirmed))
                    return false;
            }
            else
                return false;
        }
    }

    return true;
}

/**
 * Reads the state of the VAD for the given index into mState and
 * returns true.  Returns false if EOD.
 */
bool Tracter::VADGate::readVADState(IndexType iIndex)
{
    assert(iIndex >= 0);
    CacheArea vadArea;
    if (mVADInput->Read(vadArea, iIndex) == 0)
    {
        Verbose(1, "readVADState: End Of Data at %ld\n", iIndex);
        mUpstreamEndOfData = true;
        return false;
    }
    VADState* state = mVADInput->GetPointer(vadArea.offset);
    mState = *state;
    return true;
}

/**
 * Assuming we are in state SILENCE_CONFIRMED, typically at the
 * beginning of a file, advance to state SPEECH_CONFIRMED.  This will
 * set values for mSpeechTriggered and mSpeechConfirmed, then return
 * true.  Returns false if no speech could be found.
 */
bool Tracter::VADGate::confirmSpeech(IndexType iIndex)
{
    Verbose(1, "Attempting to confirm speech\n");
    assert(iIndex >= 0);
    assert(mState == SILENCE_CONFIRMED);

    IndexType index = iIndex - 1;
    do
    {
        do
        {
            if (!readVADState(++index))
                return false;
            assert((mState == SILENCE_CONFIRMED) ||
                   (mState == SPEECH_TRIGGERED));
        }
        while (mState == SILENCE_CONFIRMED);
        assert(mState == SPEECH_TRIGGERED);
        mSpeechTriggered = index;
        Verbose(1, "confirmSpeech: triggered at %ld\n", mSpeechTriggered);

        do
        {
            if (!readVADState(++index))
                return false;
        }
        while (mState == SPEECH_TRIGGERED);
    }
    while (mState != SPEECH_CONFIRMED);
    mSpeechConfirmed = index;

    Verbose(1, "confirmSpeech: confirmed at %ld\n", mSpeechConfirmed);
    return true;
}

/**
 * When the state falls to SILENCE_TRIGGERED, this returns true if it
 * subsequently goes back to SPEECH_CONFIRMED.  The other option is
 * return false if silence is confirmed.
 */
bool Tracter::VADGate::reconfirmSpeech(IndexType iIndex)
{
    Verbose(1, "Attempting to reconfirm speech\n");
    assert(iIndex >= 0);
    assert(mState == SILENCE_TRIGGERED);
    do
    {
        if (!readVADState(++iIndex))
        {
            mSilenceConfirmed = iIndex + 1;
            return false;
        }
    }
    while (mState == SILENCE_TRIGGERED);

    if (mState == SPEECH_CONFIRMED)
    {
        mSpeechConfirmed = iIndex;
        Verbose(1, "reconfirmSpeech: confirmed at %ld\n", mSpeechConfirmed);
        return true;
    }

    assert(mState == SILENCE_CONFIRMED);
    mSilenceConfirmed = iIndex;
    Verbose(1, "reconfirmSpeech: Silence confirmed at %ld\n",
            mSilenceConfirmed);

    return false;
}
