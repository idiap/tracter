/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ViterbiVAD.h"
#include "math.h"

Tracter::ViterbiVAD::ViterbiVAD(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mFrame.size = 1;

    // Viterbi config - all time values are in frames
    mInput = iInput;
    mLookAhead = config("Lookahead", 50);
    mSilStates = config("MinSilDur",20);
    mSpeechStates = config("MinSpeechDur",20);
    mSilPrior = config("SilPrior",0.8f);
    mSpeechPrior = 1.0 - mSilPrior;
    mInsPen = config("InsPen",-40);
    
    // some sanity checks
    assert(mSilPrior > 0 && mSilPrior < 1);
    assert(mSilStates > 0 && mSpeechStates > 0);
    assert(mLookAhead >= 0 );

    mEndSpeech = mSilStates + mSpeechStates - 1;
    mEndSil = mSilStates - 1;
    mIndex = 0;
    mLookAheadIndex = -1;
    mEndOfData = -1;

    score.resize(mSilStates+mSpeechStates,0.0);
    tmp_score.resize(mSilStates+mSpeechStates,0.0);
    
    Connect(iInput,mLookAhead);
}


/**
 * Catch reset.  Whether to pass upstream is an option.  In an online
 * mode, it shouldn't be passed on, but when the input is a sequence
 * of files it should be.
 */
void Tracter::ViterbiVAD::Reset(bool iPropagate)
{
    Verbose(2, "Resetting\n");

    mIndex = 0;
    mLookAheadIndex = -1;
    mEndOfData = -1;

    traceback.clear();
}

bool Tracter::ViterbiVAD::UnaryFetch(IndexType iIndex, VADState* oData)
{
    assert(iIndex >= 0);
    assert(oData);

    // Copy input to output
    if (getVADState(iIndex) == 0)
        return false;

    *oData = mState;

    return true;
}

/**
 * Reads the state of the VAD for the given index into mState and
 * returns true.  Returns false if EOD.
 */
bool Tracter::ViterbiVAD::getVADState(IndexType iIndex)
{
    assert(iIndex >= 0);

    CacheArea inputArea;
    if (iIndex == 0){ // special case to take into account Viterbi lookahead
      // first get the lookahead working
      for (int i = 0; i < mLookAhead-1; i++){
	//printf("Read input %i\n",i); fflush(stdout);
	if (mInput->Read(inputArea, i) == 0){
	  Verbose(2, "getVADState: End Of Data at %ld\n", i);
	  return false;
	}
	assert(inputArea.Length() == 1);
	float* p = mInput->GetPointer(inputArea.offset);
	//printf("SilProb[%i] %f\n",i,*p);
	doForward(i,*p);
	VADState v;
	doTraceback(0,v);
      }
    }else if (iIndex >= mEndOfData && mEndOfData >= 0){
      return false;
    }

    //printf("Read input %i\n",iIndex+mLookAhead-1); fflush(stdout);
    if (mEndOfData >= 0){
      // do nothing
    }else if (mInput->Read(inputArea, iIndex+mLookAhead-1) == 0){
      Verbose(2, "getVADState: End Of Data at %ld\n", iIndex+mLookAhead-1);
      mEndOfData = iIndex+mLookAhead-1;
    }else{
      assert(inputArea.Length() == 1);
      float* p = mInput->GetPointer(inputArea.offset);
      //printf("SilProb[%i] %f\n",iIndex+mLookAhead-1,*p);
      doForward(iIndex+mLookAhead-1,*p);
    }

    VADState v;
    if (doTraceback(iIndex,v)){
      mState = v;
    }else{
      return false;
    }

    return true;
}

//  makes the traceback of the viterbi decoding given the starting point 
//  (max_index is index of max scoring path)
bool Tracter::ViterbiVAD::doTraceback(IndexType iIndex, VADState &vad_state){
  assert(iIndex <= mLookAheadIndex && iIndex >= mIndex);

  int tb_index = mBestState;
  //printf("    tb_index: %i -> %i\n",mLookAheadIndex,tb_index); fflush(stdout);

  if (traceback.size() == 0)
    return false;

  IndexType i = mLookAheadIndex;
  for (std::deque< std::vector<int> >::reverse_iterator it=traceback.rbegin(); 
       it != traceback.rend() && i != iIndex;
       it++, i--){
    tb_index = (*it)[tb_index];
    //printf("    tb_index: %i -> %i\n",i-1,tb_index); fflush(stdout);
  }
 
  if (tb_index >= mSilStates){
    vad_state = SPEECH_TRIGGERED;
    //printf("  traceback at %i is SPEECH\n", i); fflush(stdout);
  }else{
    vad_state = SILENCE_TRIGGERED;
    //printf("  traceback at %i is SILENCE\n", i); fflush(stdout);
  }

  return true;
}


// this advances the viterbi search forward one frame using the given silence posterior probability
void Tracter::ViterbiVAD::doForward(IndexType iIndex, float pSil){
  // check that iIndex is one frame advanced from last one (or reset has been previously called)
  if (mLookAheadIndex == -1){
    mLookAheadIndex = iIndex;
    mIndex = iIndex;
  }else{
    mLookAheadIndex++;
  }
  assert(iIndex == mLookAheadIndex);

  // calculate scaled likelihoods
  float lSil = logf(pSil/mSilPrior);
  float lSpeech = logf((1.0-pSil)/mSpeechPrior);

  //  printf("============== %i ===============\n",iIndex);
  //printf("iIndex: %i   lSil: %f     lSpeech %f\n",iIndex,lSil,lSpeech);

  // take care of the traceback first (if traceback history is full, we use pop/push as in a circular buffer, otherwise add new traceback)
  if (mLookAhead == (int)traceback.size()){
    //printf("tb: circular buffer\n");
    mIndex++;
    std::vector<int> &tmp_tb = traceback.front();
    traceback.pop_front();
    traceback.push_back(tmp_tb);
    //traceback.push_back(std::vector<int>(mSilStates+mSpeechStates));
  }else if (pSil < 0 && traceback.size() > 0){
    //printf("tb: end buffer\n");
    mIndex++;
    traceback.pop_front();
    return;
  }else{
    //printf("tb: new buffer\n");
    //traceback.push_back(*(new std::vector<int>(mSilStates+mSpeechStates)));
    traceback.push_back(std::vector<int>(mSilStates+mSpeechStates));
  }
  std::vector<int> &tb = traceback.back();
 
  // process likelihoods of Sil states
  float tmp_score_a = lSil + score[0];
  float tmp_score_b = lSil + mInsPen + score[mEndSpeech];
  if (tmp_score_a >= tmp_score_b){
    tmp_score[0] = tmp_score_a; 
    tb[0] = 0;
  }else{
    tmp_score[0] = tmp_score_b;
    tb[0] = mEndSpeech;
  }
  float max_score = tmp_score[0];
  mBestState = 0;

  for (int i = 1; i < mSilStates; i++){
    tmp_score[i] = lSil + score[i-1];
    tb[i] = i-1;

    //tmp_score_a = lSil + score[i];
    //tmp_score_b = lSil + score[i-1];
    //if (tmp_score_a >= tmp_score_b){
    //  tmp_score[i] = tmp_score_a;
    //  tb[i] = i;
    //}else{
    //  tmp_score[0] = tmp_score_b;
    //  tb[i] = i-1;
    //}

    if (tmp_score[i] > max_score){
      max_score = tmp_score[i];
      mBestState = tb[i];
    }
  }

  // process likelihoods of Speech states
  tmp_score_a = lSpeech + score[mSilStates];
  tmp_score_b = lSpeech + mInsPen + score[mEndSil];
  if (tmp_score_a >= tmp_score_b){
    tmp_score[mSilStates] = tmp_score_a;
    tb[mSilStates] = mSilStates;
  }else{
    tmp_score[mSilStates] = tmp_score_b;
    tb[mSilStates] = mEndSil;
  }
  if (tmp_score[mSilStates] > max_score){
      max_score = tmp_score[mSilStates];
      mBestState = mSilStates;
  }

  for (int i = mSilStates+1; i < mSilStates+mSpeechStates; i++){
    tmp_score[i] = lSpeech + score[i-1];
    tb[i] = i-1;

    //tmp_score_a = lSpeech + score[i];
    //tmp_score_b = lSpeech + score[i-1];
    //if (tmp_score_a >= tmp_score_b){
    //  tmp_score[i] = tmp_score_a;
    //  tb[i] = i;
    //}else{
    //  tmp_score[0] = tmp_score_b;
    //  tb[i] = i-1;
    //}

    if (tmp_score[i] > max_score){
      max_score = tmp_score[i];
      mBestState = tb[i];
    }
  }

  // move the new scores into their proper place
  score.swap(tmp_score);

  //for (int i = 0; i < mSilStates + mSpeechStates; i++){
  //  printf("score[%i]: %f\n",i,score[i]);
  //}
  //printf("max index: %i score %f\n",mBestState,max_score);

  //VADState vad_state;
  //doTraceback(iIndex, vad_state);

  return;
}
