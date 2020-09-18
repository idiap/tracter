/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ViterbiVADGate.h"

Tracter::ViterbiVADGate::ViterbiVADGate(
    Component<float>* iInput,
    ViterbiVAD* iVADInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mFrame.size = iInput->frame().size;

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
    mCollar = config("Collar", 0);

    assert(mCollar >= 0);

    connect(iInput,mCollar+1);
    //connect(iVADInput, mCollar+1);
    connect(iVADInput, mCollar+10);
}

/**
 * Catch reset.  Whether to pass upstream is an option.  In an online
 * mode, it shouldn't be passed on, but when the input is a sequence
 * of files it should be.
 */
void Tracter::ViterbiVADGate::reset(bool iPropagate)
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

bool Tracter::ViterbiVADGate::unaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    assert(oData);

    //    printf("-----------> requested downstream input from %i\n",iIndex);

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

    //    printf("<---------- requested upstream input from %i\n",iIndex);

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
bool Tracter::ViterbiVADGate::gate(IndexType& iIndex)
{
  assert(iIndex >= 0);
  assert(mIndexZero >= 0);

  // If the VAD has found an utterance, it must be reset before
  // looking for another
  if (mSilenceConfirmed >= 0 && !mRemoveSilence){
    return false;
  }

  // iIndex is from the downstream point of view.  Reality could be ahead.
  //printf("original iIndex: %i\n",iIndex);
  iIndex += mIndexZero;

  //printf("upstream iIndex: %i\n",iIndex);

  if ((mSpeechTriggered < 0) && !confirmSpeech(iIndex)){
    // Failed to find any speech
    return false;
  }

  assert(mState == SPEECH_CONFIRMED);
  assert(mSpeechTriggered >= 0);
  assert(mSpeechConfirmed >= 0);

  //printf("mSpeechConfirmed %i, mSpeechRemoved %i, mIndexZero %i\n",mSpeechConfirmed,mSpeechRemoved,mIndexZero);
  iIndex += mSpeechConfirmed - mSpeechRemoved - mIndexZero;
  //printf("adjusted iIndex: %i\n",iIndex);
  if ((iIndex > mSpeechTriggered)){
    if (!readVADState(iIndex)){
      mSilenceConfirmed = iIndex + 1;
      return false;
    }
    if ((mState == SILENCE_TRIGGERED) && !reconfirmSpeech(iIndex)){
      assert(mSilenceConfirmed >= mSpeechTriggered);
      if (mRemoveSilence && (mState == SILENCE_CONFIRMED)){
	mSpeechRemoved += mSilenceConfirmed - mSpeechConfirmed;
	if (!confirmSpeech(mSilenceConfirmed+1)){
	  mSilenceConfirmed++;
	  return false;
	}else{
	  iIndex += mSpeechConfirmed - mSilenceConfirmed;
	}
      }else{
	return false;
      }
    }
  }

  //printf("mState: %i\n",mState); fflush(stdout);

  return true;
}

/**
 * Reads the state of the VAD for the given index into mState and
 * returns true.  Returns false if EOD.
 */
bool Tracter::ViterbiVADGate::readVADState(IndexType iIndex)
{
    assert(iIndex >= 0);
    CacheArea vadArea;
    //printf("Reading VAD area at iIndex %i\n",iIndex);
    //printf("-----------> requested VAD input from %i\n",iIndex);
    if (mVADInput->read(vadArea, iIndex) == 0)
    {
        verbose(2, "readVADState: End Of Data at %ld\n", iIndex);
        mUpstreamEndOfData = true;
        return false;
    }
    VADState* state = mVADInput->getPointer(vadArea.offset);\
    assert(*state == SPEECH_TRIGGERED || *state == SILENCE_TRIGGERED);
    
    if (*state == SPEECH_TRIGGERED){
      if (mState != SPEECH_CONFIRMED){
	mState = SPEECH_TRIGGERED;
      }
    }else{
      if (mState != SILENCE_CONFIRMED){
	mState = SILENCE_TRIGGERED;
      }
    }

    //mState = *state;
    return true;
}

/**
 * Assuming we are in state SILENCE_CONFIRMED, typically at the
 * beginning of a file, advance to state SPEECH_CONFIRMED.  This will
 * set values for mSpeechTriggered and mSpeechConfirmed, then return
 * true.  Returns false if no speech could be found.
 */
bool Tracter::ViterbiVADGate::confirmSpeech(IndexType iIndex)
{
  verbose(2, "Attempting to confirm speech\n");
  assert(iIndex >= 0);
  assert(mState == SILENCE_CONFIRMED);

  IndexType index = iIndex - 1;
  do{
    //printf("Read VAD state at %i\n",index+1);
    if (!readVADState(++index))
      return false;
    assert((mState == SILENCE_CONFIRMED) ||
	   (mState == SPEECH_TRIGGERED) ||
	   (mState == SILENCE_TRIGGERED));
  }
  while (mState == SILENCE_CONFIRMED || mState == SILENCE_TRIGGERED);
  assert(mState == SPEECH_TRIGGERED);
  mSpeechTriggered = index;
  verbose(2, "confirmSpeech: speech triggered at %ld\n", mSpeechTriggered);

  mState = SPEECH_CONFIRMED;
  mSpeechConfirmed = index - mCollar >= 0 ? index-mCollar : 0;
  verbose(2, "confirmSpeech: speech confirmed at %ld\n", mSpeechConfirmed);
  return true;
}

/**
 * When the state falls to SILENCE_TRIGGERED, this returns true if it
 * subsequently goes back to SPEECH_CONFIRMED.  The other option is
 * return false if silence is confirmed.
 */
bool Tracter::ViterbiVADGate::reconfirmSpeech(IndexType iIndex)
{
  verbose(2, "Attempting to reconfirm speech at %ld\n",iIndex);
  assert(iIndex >= 0);
  assert(mState == SILENCE_TRIGGERED);
  IndexType mIndex = iIndex;
  do{
    if (!readVADState(++iIndex)){
      mSilenceConfirmed = iIndex + 1;
      return false;
    }
  }
  while (mState == SILENCE_TRIGGERED && iIndex - mIndex <= mCollar*2);
  // *2 since we need to take into account the collar for the next segment

  if (mState == SPEECH_TRIGGERED){
    mState = SPEECH_CONFIRMED;
    //mSpeechConfirmed = iIndex-1 - mCollar >= 0 ? iIndex-1 - mCollar : 0 ;
    mSpeechTriggered = iIndex-1;
    verbose(2, "reconfirmSpeech: speech reconfirmed at %ld\n", mSpeechTriggered);
    return true;
  }

  mState=SILENCE_CONFIRMED;
  mSilenceConfirmed = iIndex-mCollar-1;
  verbose(2, "reconfirmSpeech: silence confirmed at %ld\n",
	  mSilenceConfirmed);

  return false;
}
