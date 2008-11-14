/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/** See the file COPYING for the licence associated with this software.
 */

#include "BSAPIFilterBank.h"

Tracter::BSAPIFilterBank::BSAPIFilterBank(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    inputdim    = iInput->GetArraySize(); 

   
    SampleFreq = GetEnv("SampleFreq", 16000);
    WaveFromScaleUp = GetEnv("WaveFromScaleUp",32768);

    int VecSize    = GetEnv("VecSize", 400);
    int Step       = GetEnv("Step", 400);
    float PreemCoef  = GetEnv("PreemCoef", 0);
    int NBanks     = GetEnv("NBanks", 23);
    bool TakeLog    = GetEnv("TakeLog", 1);
    float LoFreq     = GetEnv("LoFreq", 0);
    float HiFreq     = GetEnv("HiFreq", SampleFreq/2);


    PluginObject::MinSize(mInput, 1, 1);

    mpMelBanks = static_cast<SMelBanksI *>(BSAPICreateInstance(SIID_FE_MELBANKS));
    if(!mpMelBanks)
      {
        fprintf(stderr, "No memory!");
        exit(1);
      }

    mpMelBanks->SetSampleFreq(SampleFreq);
    mpMelBanks->SetVectorSize(VecSize);
    mpMelBanks->SetStep(Step);
    mpMelBanks->SetPreemCoef(PreemCoef);
    mpMelBanks->SetNBanks(NBanks);
    mpMelBanks->SetTakeLog(TakeLog);
    mpMelBanks->SetLowFreq(LoFreq);
    mpMelBanks->SetHighFreq(HiFreq);

    mpMelBanks->SetOutBatchSize(1);
    mpMelBanks->SetTarget(&mTarget);

    mArraySize = mpMelBanks->GetNOutputs();
    assert(mArraySize > 0);

    //printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->GetArraySize(), mArraySize); 

}

Tracter::BSAPIFilterBank::~BSAPIFilterBank() throw ()
{
  mpMelBanks->Release();
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::BSAPIFilterBank::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    
    //printf("iIndex: %i\n", iIndex);

    int one = mInput->Read(inputArea, iIndex, 1);
    //    if (!one)
    // return false;

    float *pframe  = mInput->GetPointer(inputArea.offset);
 
    for (int j=0; j<one*inputdim; j++) {
      pframe[j]*=WaveFromScaleUp;
      //if ( j % 10 == 0 )  printf ("\n");
      //printf("%f ", pframe[j]);
    }
    //printf ("\n");
    
     

    if (one)
      mpMelBanks->OnWaveform( SWaveformSourceCallbackI::wfFloat , SampleFreq, 1, pframe, one * inputdim * sizeof(float), 0);
    else
      mpMelBanks->OnWaveform( SWaveformSourceCallbackI::wfFloat , SampleFreq, 1, pframe, one * inputdim * sizeof(float), PF_LASTFRAME);
 
    float* cache = GetPointer(iOffset);
    
    if (NULL != mTarget.mpNnout) {
      for (int i=0; i<mArraySize; i++) 
	cache[i] = mTarget.mpNnout[i];
         
      /*      for (int i=0; i<mArraySize; i++)
	printf("%f ",cache[i]);
	printf ("\n");*/ 
	
    }
    
      return one>=1;
}

bool Tracter::BSAPIFilterBank::STarget::OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags)
{
  

  if (nFrames){
    float *pframe = pMatrix->GetMem();
    mpNnout = pframe;
    
    /*  for (int i=0; i<nFrames; i++) {
    printf("%f ", mpNnout[i]);
  }
  printf ("\n"); 
  */

  }
 
  return true;
}
