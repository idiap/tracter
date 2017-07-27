/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "VADGate.h"

Tracter::VADGate::VADGate(
    Component<float>* iInput,
    VADStateMachine* iVADInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mFrame.size = iInput->frame().size;

    int max = std::max(iVADInput->confirmSpeechTime(),
                       iVADInput->confirmSilenceTime());
    connect(iInput);
    connect(iVADInput, max);

    mInput = iInput;
    mVADInput = iVADInput;
    mSpeechTriggered = -1;
    mSpeechConfirmed = -1;
    mSilenceConfirmed = -1;
    mIndexZero = 0;
    mSpeechRemoved = 0;
    mState = SILENCE_CONFIRMED;
    mUpstreamEndOfData = false;

    mEnabled = config("Enable", 1);
    mSegmenting = config("Segmenting", 0);
    mRemoveSilence = config("RemoveSilence", 0);
}

/**
 * Catch reset.  Whether to pass upstream is an option.  In an online
 * mode, it shouldn't be passed on, but when the input is a sequence
 * of files it should be.
 */
void Tracter::VADGate::reset(bool iPropagate)
{
    verbose(2, "Resetting\n");
    if (mSegmenting)
    {
        if (mSilenceConfirmed >= 0)
            mIndexZero = mSilenceConfirmed;
    }
    else
        mIndexZero = 0;
    mState = SILENCE_CONFIRMED;
    mSpeechTriggered = -1;
    mSpeechConfirmed = -1;
    mSilenceConfirmed = -1;
    mSpeechRemoved = 0;

    // Propagate reset upstream under these conditions
    CachedComponent<float>::reset(
        mUpstreamEndOfData ||  // Always after EOD
        !mSegmenting ||        // If not segmenting
        !mEnabled              // If disabled
    );
    mUpstreamEndOfData = false;
}

bool Tracter::VADGate::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);

    // gate() passes by reference and will update iIndex to the
    // upstream point of view.
    if (mEnabled && !gate(iIndex))
    {
        verbose(2, "gate() returned at index %ld, silConf %ld\n",
                iIndex, mSilenceConfirmed);
        if ( mSegmenting &&
             (mSilenceConfirmed >= 0) &&
             (mSilenceConfirmed <= iIndex) )
            throw Exception("iIndex ahead of silence");
        assert(
            (mSilenceConfirmed < 0) ||   /* Failed to find silence */
            (mSilenceConfirmed > iIndex) /* Succeeded */
        );

        // Must leave mIndexZero alone until reset so the downstream
        // components can query time properly
        //mIndexZero = mSilenceConfirmed;
        //mSpeechTriggered = -1;
        //mSpeechConfirmed = -1;

        return false;
    }

    // Copy input to output
    CacheArea inputArea;
    if (mInput->read(inputArea, iIndex) == 0)
        return false;

    float* input = mInput->getPointer(inputArea.offset);
    for (int i=0; i<mFrame.size; i++)
        oData[i] = input[i];

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

    // If the VAD has found an utterance, it must be reset before
    // looking for another
    if (mSilenceConfirmed >= 0)
        return false;

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
    if (mVADInput->read(vadArea, iIndex) == 0)
    {
        verbose(2, "readVADState: End Of Data at %ld\n", iIndex);
        mUpstreamEndOfData = true;
        return false;
    }
    VADState* state = mVADInput->getPointer(vadArea.offset);
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
    verbose(2, "Attempting to confirm speech\n");
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
        verbose(2, "confirmSpeech: triggered at %ld\n", mSpeechTriggered);

        do
        {
            if (!readVADState(++index))
                return false;
        }
        while (mState == SPEECH_TRIGGERED);
    }
    while (mState != SPEECH_CONFIRMED);
    mSpeechConfirmed = index;

    verbose(2, "confirmSpeech: confirmed at %ld\n", mSpeechConfirmed);
    return true;
}

/**
 * When the state falls to SILENCE_TRIGGERED, this returns true if it
 * subsequently goes back to SPEECH_CONFIRMED.  The other option is
 * return false if silence is confirmed.
 */
bool Tracter::VADGate::reconfirmSpeech(IndexType iIndex)
{
    verbose(2, "Attempting to reconfirm speech\n");
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
        verbose(2, "reconfirmSpeech: confirmed at %ld\n", mSpeechConfirmed);
        return true;
    }

    assert(mState == SILENCE_CONFIRMED);
    mSilenceConfirmed = iIndex;
    verbose(2, "reconfirmSpeech: Silence confirmed at %ld\n",
            mSilenceConfirmed);

    return false;
}
