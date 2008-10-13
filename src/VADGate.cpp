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
    //MinSize(iVADInput, 1);
    MinSize(iVADInput, 10000);

    mInput = iInput;
    mVADInput = iVADInput;
    mSpeechTriggered = -1;
    mSpeechConfirmed = -1;
    mSilenceConfirmed = -1;
    mIndexZero = 0;
    mSpeechRemoved = 0;

    mForceDecode = false;

    mEnabled = GetEnv("Enable", 1);
    mOnline = GetEnv("Online", 0);
    mPushToTalk = GetEnv("PushToTalk", 0);
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
    mSilenceConfirmed = -1;
    mSpeechRemoved = 0;
    if (!mOnline || mPushToTalk)
    {
        mIndexZero = 0;
    }
    CachedPlugin<float>::Reset(!mOnline || mPushToTalk);  // Propagate if not online
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

	//printf("%i %i\n", mSilenceConfirmed, iIndex);
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
    assert(iIndex >= 0);
    if ((mSpeechTriggered < 0) && !confirmSpeech(iIndex))
    {
        // Failed to find any speech
        return false;
    }
    iIndex += mSpeechTriggered - mSpeechRemoved - mIndexZero;
    //printf("iIndex %i, mSpeechTriggered %i mSpeechRemoved %i mIndexZero %i mSpeechConfirmed %i\n",iIndex,mSpeechTriggered,mSpeechRemoved,mIndexZero, mSpeechConfirmed); fflush(stdout);
    if ((iIndex > mSpeechConfirmed))
    {
        if (!readVADState(iIndex)){
	  mSilenceConfirmed = iIndex + 1;
	  return false;
        }
        if (mForceDecode)
        {
      	  mSilenceConfirmed = iIndex + 1;
      	  return false;
        }
        if ((mState == SILENCE_TRIGGERED) && !reconfirmSpeech(iIndex))
        {
	  assert(mSilenceConfirmed >=  mSpeechTriggered);
	  mSpeechRemoved +=  mSilenceConfirmed - mSpeechTriggered;
	  //printf("%i %i %i\n",iIndex, mSilenceConfirmed,mSpeechTriggered);
//*??*/	  if (!confirmSpeech(mSilenceConfirmed)){
              return false;
//*??*/	  }

	  //mSilenceRemoved += mSpeechTriggered - mSilenceConfirmed;
        }
    }

    return true;
}

bool Tracter::VADGate::readVADState(IndexType iIndex)
{
    assert(iIndex >= 0);
    CacheArea vadArea;
    int ret_val = mVADInput->Read(vadArea, iIndex);
    if (vadArea.forceDecode & (iIndex > 0))
    {
    	mForceDecode = true;
        mSilenceConfirmed = iIndex;
        mState = SILENCE_CONFIRMED;
        vadArea.forceDecode = false;
        Verbose(1, "button released at %ld\n", mSilenceConfirmed);
        return true;
    }
    else
    {
    	mForceDecode = false;
    }
    if (ret_val == 0) return false;
    VADState* state = mVADInput->GetPointer(vadArea.offset);
    mState = *state;
    return true;
}

bool Tracter::VADGate::confirmSpeech(IndexType iIndex)
{
    Verbose(1, "confirmSpeech entered\n");
    assert(iIndex >= 0);
//    assert((mState == SILENCE_CONFIRMED) || // Likely
//           (mState == SPEECH_TRIGGERED));   // Not sure if this can happen
    IndexType index = iIndex - 1;
    do
    {
        do
        {
	  if (!readVADState(++index) || mForceDecode){
                return false;
	  }
            assert((mState == SILENCE_CONFIRMED) ||
                   (mState == SPEECH_TRIGGERED));
        }
        while (mState == SILENCE_CONFIRMED);
        assert(mState == SPEECH_TRIGGERED);
        mSpeechTriggered = index;

        do
        {
            if (!readVADState(++index) || mForceDecode)
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
    assert(iIndex >= 0);
    assert(mState == SILENCE_TRIGGERED);
    do
    {
      if (!readVADState(++iIndex)){
	mSilenceConfirmed = iIndex + 1;
	return false;
      }
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
